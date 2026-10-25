import AsyncStorage from '@react-native-async-storage/async-storage';
import { createClient } from '@supabase/supabase-js';
import { AppState, Platform } from 'react-native';

const url = process.env.EXPO_PUBLIC_SUPABASE_URL;
const anonKey = process.env.EXPO_PUBLIC_SUPABASE_ANON_KEY;

if (!url || !anonKey) {
  throw new Error('Set EXPO_PUBLIC_SUPABASE_URL and EXPO_PUBLIC_SUPABASE_ANON_KEY in app/.env');
}

export const supabase = createClient(url, anonKey, {
  auth: {
    storage: AsyncStorage,
    autoRefreshToken: true,
    persistSession: true,
    detectSessionInUrl: false,
  },
});

// Only refresh tokens on a timer while the app is in the foreground. The
// background location task refreshes on demand when it calls the API.
if (Platform.OS !== 'web') {
  AppState.addEventListener('change', (state) => {
    if (state === 'active') supabase.auth.startAutoRefresh();
    else supabase.auth.stopAutoRefresh();
  });
}

export type Profile = { id: string; username: string; display_name: string };

export type Settings = { sharing_enabled: boolean; radius_m: number };

export type Friend = {
  friend_id: string;
  username: string;
  display_name: string;
  status: 'accepted' | 'incoming' | 'outgoing';
  i_share: boolean;
  they_share: boolean;
};

export type NearbyFriend = {
  friend_id: string;
  username: string;
  display_name: string;
  approx_distance_m: number;
  updated_at: string;
};
