import * as Location from 'expo-location';
import * as TaskManager from 'expo-task-manager';

import { supabase } from './supabase';

export const BACKGROUND_LOCATION_TASK = 'friend-radar-location';

// Round to 3 decimal places (~110 m) so a precise location never leaves the
// phone. The server snaps again in case an older client doesn't.
export function snap(value: number) {
  return Math.round(value * 1000) / 1000;
}

export async function reportLocation(coords: { latitude: number; longitude: number }) {
  const { data } = await supabase.auth.getSession();
  if (!data.session) return;
  const { error } = await supabase.functions.invoke('report-location', {
    body: { lat: snap(coords.latitude), lng: snap(coords.longitude) },
  });
  if (error) console.warn('report-location failed', error.message);
}

export async function reportCurrentLocation() {
  const { status } = await Location.getForegroundPermissionsAsync();
  if (status !== 'granted') return;
  const position = await Location.getCurrentPositionAsync({
    accuracy: Location.Accuracy.Balanced,
  });
  await reportLocation(position.coords);
}

// Must be defined at module scope so it is registered when the OS launches the
// app in the background. Imported from the root layout.
TaskManager.defineTask<{ locations: Location.LocationObject[] }>(
  BACKGROUND_LOCATION_TASK,
  async ({ data, error }) => {
    if (error) {
      console.warn('background location error', error.message);
      return;
    }
    const latest = data?.locations?.at(-1);
    if (latest) await reportLocation(latest.coords);
  },
);

export type PermissionLevel = 'none' | 'foreground' | 'background';

export async function getPermissionLevel(): Promise<PermissionLevel> {
  const fg = await Location.getForegroundPermissionsAsync();
  if (fg.status !== 'granted') return 'none';
  const bg = await Location.getBackgroundPermissionsAsync();
  return bg.status === 'granted' ? 'background' : 'foreground';
}

// Ask for "while using" first, then "always". Both platforms require that order.
export async function requestPermissions(): Promise<PermissionLevel> {
  const fg = await Location.requestForegroundPermissionsAsync();
  if (fg.status !== 'granted') return 'none';
  const bg = await Location.requestBackgroundPermissionsAsync();
  return bg.status === 'granted' ? 'background' : 'foreground';
}

export async function startBackgroundUpdates() {
  if (await Location.hasStartedLocationUpdatesAsync(BACKGROUND_LOCATION_TASK)) return;
  await Location.startLocationUpdatesAsync(BACKGROUND_LOCATION_TASK, {
    // Balanced accuracy plus a 100 m distance filter is plenty for a ~110 m
    // grid, and is much easier on the battery than GPS-level accuracy.
    accuracy: Location.Accuracy.Balanced,
    distanceInterval: 100,
    timeInterval: 60_000,
    deferredUpdatesInterval: 60_000,
    pausesUpdatesAutomatically: true,
    activityType: Location.ActivityType.Other,
    showsBackgroundLocationIndicator: false,
    foregroundService: {
      notificationTitle: 'Friend Radar is sharing your location',
      notificationBody: 'Friends you picked can see roughly how far away you are.',
      killServiceOnDestroy: false,
    },
  });
}

export async function stopBackgroundUpdates() {
  if (await Location.hasStartedLocationUpdatesAsync(BACKGROUND_LOCATION_TASK)) {
    await Location.stopLocationUpdatesAsync(BACKGROUND_LOCATION_TASK);
  }
}
