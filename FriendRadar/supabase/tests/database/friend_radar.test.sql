-- Run with: supabase test db
begin;
create extension if not exists pgtap with schema extensions;
set search_path = public, extensions;

select plan(25);

-- Users ----------------------------------------------------------------------
insert into auth.users (id, email) values
  ('00000000-0000-0000-0000-00000000000a', 'alice@example.com'),
  ('00000000-0000-0000-0000-00000000000b', 'bob@example.com'),
  ('00000000-0000-0000-0000-00000000000c', 'carol@example.com'),
  ('00000000-0000-0000-0000-00000000000d', 'dave@example.com'),
  ('00000000-0000-0000-0000-00000000000e', 'erin@example.com');

create function pg_temp.login(uid uuid) returns void language sql as $$
  select set_config('role', 'authenticated', true),
         set_config('request.jwt.claims', json_build_object('sub', uid, 'role', 'authenticated')::text, true);
$$;
create function pg_temp.logout() returns void language sql as $$
  select set_config('role', 'postgres', true), set_config('request.jwt.claims', '', true);
$$;

select pg_temp.login('00000000-0000-0000-0000-00000000000a');
insert into profiles (id, username, display_name) values ('00000000-0000-0000-0000-00000000000a', 'alice', 'Alice');
select pg_temp.login('00000000-0000-0000-0000-00000000000b');
insert into profiles (id, username, display_name) values ('00000000-0000-0000-0000-00000000000b', 'bob', 'Bob');
select pg_temp.login('00000000-0000-0000-0000-00000000000c');
insert into profiles (id, username, display_name) values ('00000000-0000-0000-0000-00000000000c', 'carol', 'Carol');
select pg_temp.login('00000000-0000-0000-0000-00000000000d');
insert into profiles (id, username, display_name) values ('00000000-0000-0000-0000-00000000000d', 'dave', 'Dave');

select throws_ok(
  $$insert into profiles (id, username, display_name) values ('00000000-0000-0000-0000-00000000000e', 'mallory', 'M')$$,
  '42501', null, 'cannot create a profile for someone else'
);

-- Friend requests -------------------------------------------------------------
select pg_temp.login('00000000-0000-0000-0000-00000000000a');
select is(send_friend_request('bob'), 'requested', 'alice requests bob');
select is(send_friend_request('carol'), 'requested', 'alice requests carol');
select is((select count(*) from profiles where username = 'dave'), 0::bigint, 'strangers are hidden');

select pg_temp.login('00000000-0000-0000-0000-00000000000b');
select is((select status from my_friends() where username = 'alice'), 'incoming', 'bob sees incoming request');
select lives_ok($$select respond_friend_request('00000000-0000-0000-0000-00000000000a', true)$$, 'bob accepts');
select pg_temp.login('00000000-0000-0000-0000-00000000000c');
select is(send_friend_request('alice'), 'accepted', 'requesting back accepts');

-- Sharing is opt-in -----------------------------------------------------------
select pg_temp.logout();
select is(
  (select count(*) from report_location('00000000-0000-0000-0000-00000000000a', 40.7, -74.0)),
  0::bigint, 'nothing stored while sharing is off'
);
select is((select count(*) from locations), 0::bigint, 'no location row while sharing is off');

-- Everyone turns sharing on with a 1 km radius.
update user_settings set sharing_enabled = true, radius_m = 1000;

-- Alice shares with bob and carol; bob shares with alice; carol does not.
select pg_temp.login('00000000-0000-0000-0000-00000000000a');
select set_share_with_friend('00000000-0000-0000-0000-00000000000b', true);
select set_share_with_friend('00000000-0000-0000-0000-00000000000c', true);
select pg_temp.login('00000000-0000-0000-0000-00000000000b');
select set_share_with_friend('00000000-0000-0000-0000-00000000000a', true);

-- Locations and alerts ---------------------------------------------------------
select pg_temp.logout();
select is(
  (select count(*) from report_location('00000000-0000-0000-0000-00000000000a', 40.70001, -74.00002)),
  0::bigint, 'first report: nobody else located yet'
);
select is(
  (select st_y(geog::geometry) from locations where user_id = '00000000-0000-0000-0000-00000000000a'),
  40.7::double precision, 'stored location is snapped to the grid'
);
-- Carol is right next to alice but doesn't share back.
select is(
  (select count(*) from report_location('00000000-0000-0000-0000-00000000000c', 40.7001, -74.0)),
  0::bigint, 'no alert without mutual sharing'
);
-- Bob arrives ~550 m away: both alice and bob get one alert.
select results_eq(
  $$select recipient_id::text, subject_name, approx_distance_m
    from report_location('00000000-0000-0000-0000-00000000000b', 40.705, -74.0)
    order by subject_name$$,
  $$values ('00000000-0000-0000-0000-00000000000b', 'Alice', 750),
           ('00000000-0000-0000-0000-00000000000a', 'Bob', 750)$$,
  'entering the radius alerts both sides'
);
select is(
  (select count(*) from report_location('00000000-0000-0000-0000-00000000000b', 40.705, -74.0)),
  0::bigint, 'no repeat alert while still inside'
);

-- Bob leaves (3 km away) and comes back: state resets, but the cooldown stops a
-- second alert so soon after the first.
select is(
  (select count(*) from report_location('00000000-0000-0000-0000-00000000000b', 40.73, -74.0)),
  0::bigint, 'leaving the radius does not alert'
);
select is(
  (select count(*) from proximity_state where inside),
  0::bigint, 'leaving marks both directions outside'
);
select is(
  (select count(*) from report_location('00000000-0000-0000-0000-00000000000b', 40.705, -74.0)),
  0::bigint, 're-entering within the cooldown does not alert again'
);

select pg_temp.login('00000000-0000-0000-0000-00000000000a');
select results_eq(
  $$select username, approx_distance_m from nearby_friends()$$,
  $$values ('bob'::text, 750)$$,
  'alice sees only bob, at a rounded distance'
);

-- Radius is per-viewer: alice shrinks hers and no longer sees bob; bob still sees alice.
update user_settings set radius_m = 300;
select is((select count(*) from nearby_friends()), 0::bigint, 'outside my radius');
select pg_temp.login('00000000-0000-0000-0000-00000000000b');
select is((select count(*) from nearby_friends()), 1::bigint, 'bob still sees alice in his radius');

-- Clients can't touch raw data or server functions.
select throws_ok($$select * from locations$$, '42501', null, 'locations are not readable');
select throws_ok(
  $$select * from report_location('00000000-0000-0000-0000-00000000000b', 0, 0)$$,
  '42501', null, 'clients cannot call report_location'
);
select throws_ok(
  $$update user_settings set push_token = 'x'$$, '42501', null, 'push token is not directly writable'
);

-- Bob stops sharing with alice: invisible to each other right away.
select set_share_with_friend('00000000-0000-0000-0000-00000000000a', false);
select is((select count(*) from nearby_friends()), 0::bigint, 'unsharing hides immediately');

-- Master switch off deletes the stored location.
update user_settings set sharing_enabled = false;
select pg_temp.logout();
select is(
  (select count(*) from locations where user_id = '00000000-0000-0000-0000-00000000000b'),
  0::bigint, 'turning sharing off deletes my location'
);

select * from finish();
rollback;
