// Receives a location update from the app, stores it via report_location(),
// and sends Expo push notifications for any friend who just came into range.
import { createClient } from "npm:@supabase/supabase-js@2";

const EXPO_PUSH_URL = "https://exp.host/--/api/v2/push/send";
const EXPO_BATCH_SIZE = 100;

type Alert = {
  recipient_id: string;
  subject_id: string;
  subject_name: string;
  approx_distance_m: number;
};

type ExpoTicket = { status: "ok" | "error"; details?: { error?: string } };

const admin = createClient(
  Deno.env.get("SUPABASE_URL")!,
  Deno.env.get("SUPABASE_SERVICE_ROLE_KEY")!,
  { auth: { persistSession: false, autoRefreshToken: false } },
);

function json(body: unknown, status = 200) {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "Content-Type": "application/json" },
  });
}

function formatDistance(meters: number) {
  return meters < 1000 ? `${meters} m` : `${(meters / 1000).toFixed(meters < 10000 ? 1 : 0)} km`;
}

function isCoordinate(value: unknown, limit: number): value is number {
  return typeof value === "number" && Number.isFinite(value) && Math.abs(value) <= limit;
}

async function sendPushes(alerts: Alert[]) {
  const recipients = [...new Set(alerts.map((a) => a.recipient_id))];
  const { data: rows, error } = await admin
    .from("user_settings")
    .select("user_id, push_token")
    .in("user_id", recipients)
    .not("push_token", "is", null);
  if (error) throw error;

  const tokenByUser = new Map(rows.map((r) => [r.user_id as string, r.push_token as string]));
  const messages = alerts
    .filter((a) => tokenByUser.has(a.recipient_id))
    .map((a) => ({
      to: tokenByUser.get(a.recipient_id)!,
      title: `${a.subject_name} is nearby`,
      body: `About ${formatDistance(a.approx_distance_m)} away`,
      sound: "default",
      channelId: "nearby",
      data: { type: "friend_nearby", friendId: a.subject_id },
    }));

  const headers: Record<string, string> = { "Content-Type": "application/json" };
  const accessToken = Deno.env.get("EXPO_ACCESS_TOKEN");
  if (accessToken) headers.Authorization = `Bearer ${accessToken}`;

  for (let i = 0; i < messages.length; i += EXPO_BATCH_SIZE) {
    const batch = messages.slice(i, i + EXPO_BATCH_SIZE);
    const res = await fetch(EXPO_PUSH_URL, { method: "POST", headers, body: JSON.stringify(batch) });
    if (!res.ok) {
      console.error("Expo push failed", res.status, await res.text());
      continue;
    }
    // Tickets come back in the same order as the messages. Forget tokens for
    // uninstalled apps so we stop sending to them.
    const { data: tickets } = (await res.json()) as { data: ExpoTicket[] };
    const dead = batch
      .filter((_, j) => tickets[j]?.details?.error === "DeviceNotRegistered")
      .map((m) => m.to);
    if (dead.length) {
      await admin.from("user_settings").update({ push_token: null }).in("push_token", dead);
    }
  }
}

Deno.serve(async (req) => {
  if (req.method !== "POST") return json({ error: "Method not allowed" }, 405);

  const jwt = (req.headers.get("Authorization") ?? "").replace(/^Bearer\s+/i, "");
  if (!jwt) return json({ error: "Unauthorized" }, 401);
  const { data: auth, error: authError } = await admin.auth.getUser(jwt);
  if (authError || !auth.user) return json({ error: "Unauthorized" }, 401);

  let body: { lat?: unknown; lng?: unknown };
  try {
    body = await req.json();
  } catch {
    return json({ error: "Invalid JSON" }, 400);
  }
  if (!isCoordinate(body.lat, 90) || !isCoordinate(body.lng, 180)) {
    return json({ error: "lat and lng are required" }, 400);
  }

  const { data: alerts, error } = await admin.rpc("report_location", {
    p_user: auth.user.id,
    p_lat: body.lat,
    p_lng: body.lng,
  });
  if (error) {
    console.error("report_location failed", error);
    return json({ error: "Could not save location" }, 500);
  }

  if (alerts?.length) {
    try {
      await sendPushes(alerts as Alert[]);
    } catch (e) {
      // The location is saved; a failed push shouldn't fail the request.
      console.error("sending pushes failed", e);
    }
  }

  return json({ ok: true });
});
