-- Friend Radar: opt-in, mutual, approximate proximity sharing between friends.
--
-- Privacy model
--   * Locations are snapped to a ~110 m grid before they are stored, and the
--     locations table is never readable by clients. Friends only ever see a
--     rounded distance (see approx_distance_m), never coordinates.
--   * A and B can see each other only when: they are accepted friends, both have
--     the master sharing switch on, A shares with B and B shares with A.
--   * Each user only sees friends inside their own chosen radius.

create extension if not exists postgis with schema extensions;

-------------------------------------------------------------------------------
-- Tables
-------------------------------------------------------------------------------

create table public.profiles (
  id uuid primary key references auth.users (id) on delete cascade,
  username text not null unique check (username ~ '^[a-z0-9_]{3,20}$'),
  display_name text not null check (char_length(display_name) between 1 and 40),
  created_at timestamptz not null default now()
);

-- Private per-user settings. Only the owner can read or change them.
create table public.user_settings (
  user_id uuid primary key references public.profiles (id) on delete cascade,
  sharing_enabled boolean not null default false,
  radius_m integer not null default 1000 check (radius_m between 100 and 50000),
  push_token text,
  updated_at timestamptz not null default now()
);

-- Directed friendship rows. A request from A to B is (A, B, 'pending'); once B
-- accepts there are two rows, (A, B) and (B, A), both 'accepted'.
-- shares_location on (owner, viewer) means owner lets viewer see them.
create table public.friendships (
  user_id uuid not null references public.profiles (id) on delete cascade,
  friend_id uuid not null references public.profiles (id) on delete cascade,
  status text not null check (status in ('pending', 'accepted')),
  shares_location boolean not null default false,
  created_at timestamptz not null default now(),
  primary key (user_id, friend_id),
  check (user_id <> friend_id)
);
create index friendships_friend_idx on public.friendships (friend_id);

-- Last known (snapped) location. No client access at all.
create table public.locations (
  user_id uuid primary key references public.profiles (id) on delete cascade,
  geog extensions.geography(point, 4326) not null,
  updated_at timestamptz not null default now()
);
create index locations_geog_idx on public.locations using gist (geog);

-- Whether subject is currently inside recipient's radius, so we only alert on
-- entry, with a cooldown to stop alerts flapping at the boundary.
create table public.proximity_state (
  recipient_id uuid not null references public.profiles (id) on delete cascade,
  subject_id uuid not null references public.profiles (id) on delete cascade,
  inside boolean not null default false,
  last_alerted_at timestamptz,
  primary key (recipient_id, subject_id)
);

-------------------------------------------------------------------------------
-- Row level security
-------------------------------------------------------------------------------

alter table public.profiles enable row level security;
alter table public.user_settings enable row level security;
alter table public.friendships enable row level security;
alter table public.locations enable row level security;
alter table public.proximity_state enable row level security;

-- Profiles are visible to yourself and to anyone you have a friendship row with
-- (in either direction, including pending requests).
create policy profiles_select on public.profiles for select to authenticated
  using (
    id = (select auth.uid())
    or exists (
      select 1 from public.friendships f
      where (f.user_id = (select auth.uid()) and f.friend_id = profiles.id)
         or (f.friend_id = (select auth.uid()) and f.user_id = profiles.id)
    )
  );
create policy profiles_insert on public.profiles for insert to authenticated
  with check (id = (select auth.uid()));
create policy profiles_update on public.profiles for update to authenticated
  using (id = (select auth.uid())) with check (id = (select auth.uid()));

create policy settings_select on public.user_settings for select to authenticated
  using (user_id = (select auth.uid()));
create policy settings_update on public.user_settings for update to authenticated
  using (user_id = (select auth.uid())) with check (user_id = (select auth.uid()));

-- Friendships are read-only for clients; all changes go through the functions
-- below so the two directed rows stay consistent.
create policy friendships_select on public.friendships for select to authenticated
  using (user_id = (select auth.uid()) or friend_id = (select auth.uid()));

-- locations and proximity_state: RLS on and no policies, so clients get nothing.
revoke all on public.locations, public.proximity_state from anon, authenticated;
revoke all on public.profiles, public.user_settings, public.friendships from anon;
revoke insert, update, delete on public.friendships from authenticated;
revoke insert, delete on public.user_settings from authenticated;
-- Only these settings columns are client-editable; push_token goes via RPC too
-- so it can't be read back through a returning clause by anyone else.
revoke update on public.user_settings from authenticated;
grant update (sharing_enabled, radius_m) on public.user_settings to authenticated;

-------------------------------------------------------------------------------
-- Triggers
-------------------------------------------------------------------------------

create function public.handle_new_profile() returns trigger
language plpgsql security definer set search_path = '' as $$
begin
  insert into public.user_settings (user_id) values (new.id);
  return new;
end;
$$;

create trigger on_profile_created after insert on public.profiles
  for each row execute function public.handle_new_profile();

-- Turning the master switch off immediately forgets your location.
create function public.handle_settings_change() returns trigger
language plpgsql security definer set search_path = '' as $$
begin
  new.updated_at := now();
  if not new.sharing_enabled then
    delete from public.locations where user_id = new.user_id;
    delete from public.proximity_state
      where recipient_id = new.user_id or subject_id = new.user_id;
  end if;
  return new;
end;
$$;

create trigger on_settings_changed before update on public.user_settings
  for each row execute function public.handle_settings_change();

-------------------------------------------------------------------------------
-- Helpers
-------------------------------------------------------------------------------

-- How old a location may be and still count.
create function public.location_max_age() returns interval
language sql immutable as $$ select interval '6 hours' $$;

-- Don't re-alert about the same friend within this window.
create function public.alert_cooldown() returns interval
language sql immutable as $$ select interval '2 hours' $$;

-- Round a distance so friends get a sense of "how near", not a precise figure.
create function public.approx_distance_m(meters double precision) returns integer
language sql immutable as $$
  select case
    when meters < 1000 then greatest(250, (ceil(meters / 250) * 250))::int
    when meters < 5000 then (round(meters / 500) * 500)::int
    else (round(meters / 1000) * 1000)::int
  end
$$;

-- True when a and b may see each other (mutual opt-in, both switched on).
create function public.can_see_each_other(a uuid, b uuid) returns boolean
language sql stable security definer set search_path = '' as $$
  select exists (
    select 1
    from public.friendships ab
    join public.friendships ba on ba.user_id = b and ba.friend_id = a
    join public.user_settings sa on sa.user_id = a
    join public.user_settings sb on sb.user_id = b
    where ab.user_id = a and ab.friend_id = b
      and ab.status = 'accepted' and ba.status = 'accepted'
      and ab.shares_location and ba.shares_location
      and sa.sharing_enabled and sb.sharing_enabled
  )
$$;

-------------------------------------------------------------------------------
-- Client API (authenticated)
-------------------------------------------------------------------------------

create function public.send_friend_request(p_username text) returns text
language plpgsql security definer set search_path = '' as $$
declare
  me uuid := auth.uid();
  target uuid;
begin
  if me is null then raise exception 'not authenticated'; end if;

  select id into target from public.profiles where username = lower(trim(p_username));
  if target is null then raise exception 'No user with that username'; end if;
  if target = me then raise exception 'You cannot add yourself'; end if;

  if exists (select 1 from public.friendships where user_id = me and friend_id = target) then
    return 'already_requested';
  end if;

  -- They already asked us: treat this as accepting.
  if exists (select 1 from public.friendships
             where user_id = target and friend_id = me and status = 'pending') then
    update public.friendships set status = 'accepted'
      where user_id = target and friend_id = me;
    insert into public.friendships (user_id, friend_id, status)
      values (me, target, 'accepted');
    return 'accepted';
  end if;

  insert into public.friendships (user_id, friend_id, status)
    values (me, target, 'pending');
  return 'requested';
end;
$$;

create function public.respond_friend_request(p_from uuid, p_accept boolean) returns void
language plpgsql security definer set search_path = '' as $$
declare
  me uuid := auth.uid();
begin
  if me is null then raise exception 'not authenticated'; end if;
  if not exists (select 1 from public.friendships
                 where user_id = p_from and friend_id = me and status = 'pending') then
    raise exception 'No pending request from that user';
  end if;

  if p_accept then
    update public.friendships set status = 'accepted'
      where user_id = p_from and friend_id = me;
    insert into public.friendships (user_id, friend_id, status)
      values (me, p_from, 'accepted')
      on conflict (user_id, friend_id) do update set status = 'accepted';
  else
    delete from public.friendships where user_id = p_from and friend_id = me;
  end if;
end;
$$;

-- Also used to cancel an outgoing request.
create function public.remove_friend(p_friend uuid) returns void
language plpgsql security definer set search_path = '' as $$
declare
  me uuid := auth.uid();
begin
  if me is null then raise exception 'not authenticated'; end if;
  delete from public.friendships
    where (user_id = me and friend_id = p_friend)
       or (user_id = p_friend and friend_id = me);
  delete from public.proximity_state
    where (recipient_id = me and subject_id = p_friend)
       or (recipient_id = p_friend and subject_id = me);
end;
$$;

create function public.set_share_with_friend(p_friend uuid, p_share boolean) returns void
language plpgsql security definer set search_path = '' as $$
declare
  me uuid := auth.uid();
begin
  if me is null then raise exception 'not authenticated'; end if;
  update public.friendships set shares_location = p_share
    where user_id = me and friend_id = p_friend and status = 'accepted';
  if not found then raise exception 'Not friends with that user'; end if;
  if not p_share then
    delete from public.proximity_state
      where (recipient_id = me and subject_id = p_friend)
         or (recipient_id = p_friend and subject_id = me);
  end if;
end;
$$;

create function public.set_push_token(p_token text) returns void
language sql security definer set search_path = '' as $$
  update public.user_settings set push_token = nullif(p_token, '')
    where user_id = auth.uid()
$$;

-- Friends and requests, with sharing flags in both directions.
create function public.my_friends()
returns table (
  friend_id uuid,
  username text,
  display_name text,
  status text,          -- 'accepted' | 'incoming' | 'outgoing'
  i_share boolean,
  they_share boolean
)
language sql stable security definer set search_path = '' as $$
  with others as (
    select friend_id as id from public.friendships where user_id = auth.uid()
    union
    select user_id from public.friendships where friend_id = auth.uid()
  ),
  rows as (
    select p.id, p.username, p.display_name,
      case
        when mine.status = 'accepted' then 'accepted'
        when mine.user_id is not null then 'outgoing'
        else 'incoming'
      end as status,
      coalesce(mine.shares_location and mine.status = 'accepted', false) as i_share,
      coalesce(theirs.shares_location and theirs.status = 'accepted', false) as they_share
    from others o
    join public.profiles p on p.id = o.id
    left join public.friendships mine on mine.user_id = auth.uid() and mine.friend_id = p.id
    left join public.friendships theirs on theirs.user_id = p.id and theirs.friend_id = auth.uid()
  )
  select * from rows
  order by case status when 'incoming' then 0 when 'accepted' then 1 else 2 end, display_name
$$;

-- Friends inside my radius who mutually share with me.
create function public.nearby_friends()
returns table (
  friend_id uuid,
  username text,
  display_name text,
  approx_distance_m integer,
  updated_at timestamptz
)
language sql stable security definer set search_path = '' as $$
  select p.id, p.username, p.display_name,
    public.approx_distance_m(extensions.st_distance(mine.geog, theirs.geog)),
    -- Coarsen the timestamp too; it's only for "updated ~N min ago".
    date_trunc('minute', theirs.updated_at)
  from public.user_settings s
  join public.locations mine on mine.user_id = s.user_id
  join public.friendships f on f.user_id = s.user_id and f.status = 'accepted'
  join public.locations theirs on theirs.user_id = f.friend_id
  join public.profiles p on p.id = f.friend_id
  where s.user_id = auth.uid()
    and s.sharing_enabled
    and mine.updated_at > now() - public.location_max_age()
    and theirs.updated_at > now() - public.location_max_age()
    and extensions.st_dwithin(mine.geog, theirs.geog, s.radius_m)
    and public.can_see_each_other(s.user_id, f.friend_id)
  order by extensions.st_distance(mine.geog, theirs.geog)
$$;

-------------------------------------------------------------------------------
-- Server API (service_role only, called by the report-location edge function)
-------------------------------------------------------------------------------

-- Store a location for p_user and return the alerts that should be pushed:
-- one row per (recipient, subject) pair where subject just entered recipient's
-- radius and recipient hasn't been alerted about subject recently.
create function public.report_location(p_user uuid, p_lat double precision, p_lng double precision)
returns table (
  recipient_id uuid,
  subject_id uuid,
  subject_name text,
  approx_distance_m integer
)
language plpgsql security definer set search_path = '' as $$
#variable_conflict use_column
declare
  my_settings public.user_settings;
  snapped extensions.geography;
begin
  if p_lat not between -90 and 90 or p_lng not between -180 and 180 then
    raise exception 'invalid coordinates';
  end if;

  select * into my_settings from public.user_settings where user_id = p_user;
  if not found or not my_settings.sharing_enabled then
    return;  -- Not sharing: store nothing.
  end if;

  -- ~110 m grid. Clients snap too, this just makes sure.
  snapped := extensions.st_setsrid(
    extensions.st_makepoint(round(p_lng::numeric, 3), round(p_lat::numeric, 3)), 4326
  )::extensions.geography;

  insert into public.locations (user_id, geog, updated_at)
    values (p_user, snapped, now())
    on conflict (user_id) do update set geog = excluded.geog, updated_at = excluded.updated_at;

  return query
  with pairs as (
    -- For each mutually-sharing friend, both directions: does the friend see me
    -- (their radius), and do I see the friend (my radius)?
    select f.friend_id as friend,
           extensions.st_distance(snapped, l.geog) as dist,
           (l.updated_at > now() - public.location_max_age()) as fresh,
           fs.radius_m as friend_radius
    from public.friendships f
    join public.locations l on l.user_id = f.friend_id
    join public.user_settings fs on fs.user_id = f.friend_id
    where f.user_id = p_user and f.status = 'accepted'
      and public.can_see_each_other(p_user, f.friend_id)
  ),
  directed as (
    select friend as recipient, p_user as subject, dist, fresh, friend_radius as radius from pairs
    union all
    select p_user, friend, dist, fresh, my_settings.radius_m from pairs
  ),
  evaluated as (
    select d.recipient, d.subject, d.dist,
      coalesce(ps.inside, false) as was_inside,
      ps.last_alerted_at,
      -- Enter at radius, leave at 1.25x radius (hysteresis), stale = outside.
      case
        when not d.fresh then false
        when d.dist <= d.radius then true
        when d.dist > d.radius * 1.25 then false
        else coalesce(ps.inside, false)
      end as now_inside
    from directed d
    left join public.proximity_state ps
      on ps.recipient_id = d.recipient and ps.subject_id = d.subject
  ),
  to_alert as (
    select e.recipient, e.subject, e.dist from evaluated e
    where e.now_inside and not e.was_inside
      and (e.last_alerted_at is null or e.last_alerted_at < now() - public.alert_cooldown())
  ),
  upserted as (
    insert into public.proximity_state as ps (recipient_id, subject_id, inside, last_alerted_at)
    select e.recipient, e.subject, e.now_inside,
      case when exists (select 1 from to_alert t
                        where t.recipient = e.recipient and t.subject = e.subject)
           then now() else e.last_alerted_at end
    from evaluated e
    on conflict (recipient_id, subject_id) do update
      set inside = excluded.inside, last_alerted_at = excluded.last_alerted_at
  )
  -- (A data-modifying CTE always runs, even though nothing selects from it.)
  select t.recipient, t.subject, p.display_name, public.approx_distance_m(t.dist)
  from to_alert t
  join public.profiles p on p.id = t.subject;
end;
$$;

-------------------------------------------------------------------------------
-- Grants
-------------------------------------------------------------------------------

revoke execute on all functions in schema public from public, anon, authenticated;

grant execute on function
  public.send_friend_request(text),
  public.respond_friend_request(uuid, boolean),
  public.remove_friend(uuid),
  public.set_share_with_friend(uuid, boolean),
  public.set_push_token(text),
  public.my_friends(),
  public.nearby_friends()
to authenticated;

grant execute on function public.report_location(uuid, double precision, double precision)
  to service_role;
