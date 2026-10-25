# Friend Radar

An iOS and Android app for seeing which friends are nearby. Sharing is opt-in, works both ways, and only shows a rough distance.

- **Opt-in, per friend.** Location sharing is off until you turn it on. Then you pick which friends can see you.
- **Mutual only.** You and a friend see each other only if you both have sharing on and have both picked each other.
- **Your own radius.** Each person picks how far "nearby" reaches, from 100 m to 50 km. You only see friends inside your radius.
- **Approximate.** Your phone rounds your location to about 110 m before sending it. Friends only see a rounded distance like "~750 m", never a position on a map.
- **Background alerts.** You get a push notification when a friend comes within your radius. It fires once on entry, and the same friend can't alert you again for 2 hours.

## Layout

```
app/        Expo (React Native) app, TypeScript, Expo Router
supabase/   Postgres schema + PostGIS, row-level security, tests, Edge Function
```

How it works:

1. The phone (in the foreground, or in the background through `expo-location`) sends a rounded location to the `report-location` Edge Function.
2. The function checks the user's token and calls `report_location()` in Postgres. That stores the location and works out which friends just came into range, in either direction.
3. The function sends Expo push notifications for those alerts.
4. The Nearby tab calls `nearby_friends()`. It returns only mutually-sharing friends inside your radius, with rounded distances.

The app can never read the `locations` table directly. Only the database functions can.

## Setup

### 1. Supabase

Create a project at supabase.com, then from this folder:

```sh
npx supabase login
npx supabase link --project-ref <your-project-ref>
npx supabase db push                      # creates tables, policies and functions
npx supabase functions deploy report-location
```

Sign-in uses a 6-digit email code. `supabase/config.toml` sets up the email templates for local dev. On the hosted project, edit **Authentication → Email Templates → Magic Link** and **Confirm signup** so they include `{{ .Token }}` (see `supabase/templates/magic_link.html`).

Optional: if you turn on push security in your Expo account, set the Expo access token as a function secret:

```sh
npx supabase secrets set EXPO_ACCESS_TOKEN=...
```

### 2. App

```sh
cd app
cp .env.example .env        # fill in your Supabase URL and anon/publishable key
npm install
npx eas-cli@latest init     # writes the EAS projectId into app.json, needed for push
```

Background location and push notifications don't work in Expo Go, so you need a development build:

```sh
npx eas-cli@latest build --profile development --platform ios      # or android
npx expo start
```

Push notifications need a real device. Before your first production build, change `bundleIdentifier` / `package` in `app.json` from `com.example.friendradar` to your own.

## Tests

The database tests (pgTAP) cover mutual-only visibility, per-person radius, location rounding, alert-on-entry with cooldown, and that clients can't read raw locations:

```sh
npx supabase start
npx supabase test db
```

App checks:

```sh
cd app && npx tsc --noEmit
```

## Store review notes

Both stores look closely at background location:

- **iOS:** the "Always" permission text in `app.json` explains why the app needs it. Apple expects the app to work with "While Using" too, and this one does: without "Always" it only updates while open.
- **Android:** Google Play needs a background-location declaration form and a short video showing the feature. The app runs a foreground service with a visible notification while it shares.

## Tuning

These are in `supabase/migrations/20261025000000_init.sql`:

| Setting | Default | Function |
| --- | --- | --- |
| How long a location stays valid | 6 hours | `location_max_age()` |
| Gap between alerts for the same friend | 2 hours | `alert_cooldown()` |
| Distance rounding | 250 m steps under 1 km, 500 m under 5 km, then 1 km | `approx_distance_m()` |

On the phone, background updates fire after about 100 m of movement (`app/src/lib/location.ts`).
