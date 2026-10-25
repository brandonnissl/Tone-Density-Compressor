import type { Session } from '@supabase/supabase-js';
import { createContext, useCallback, useContext, useEffect, useState, type ReactNode } from 'react';

import {
  getPermissionLevel,
  reportCurrentLocation,
  startBackgroundUpdates,
  stopBackgroundUpdates,
} from './location';
import { clearPushToken, registerForPushNotifications } from './notifications';
import { supabase, type Profile, type Settings } from './supabase';

type SessionState = {
  loading: boolean;
  session: Session | null;
  profile: Profile | null;
  settings: Settings | null;
  refresh: () => Promise<void>;
  updateSettings: (patch: Partial<Settings>) => Promise<void>;
  signOut: () => Promise<void>;
};

const SessionContext = createContext<SessionState | null>(null);

export function useSession() {
  const value = useContext(SessionContext);
  if (!value) throw new Error('useSession must be used inside SessionProvider');
  return value;
}

// Make background tracking match the sharing switch and the permission the
// user actually granted.
async function syncTracking(sharing: boolean) {
  const level = await getPermissionLevel();
  if (sharing && level === 'background') {
    await startBackgroundUpdates();
  } else {
    await stopBackgroundUpdates();
  }
  if (sharing && level !== 'none') await reportCurrentLocation();
}

export function SessionProvider({ children }: { children: ReactNode }) {
  const [loading, setLoading] = useState(true);
  const [session, setSession] = useState<Session | null>(null);
  const [profile, setProfile] = useState<Profile | null>(null);
  const [settings, setSettings] = useState<Settings | null>(null);

  const loadUserData = useCallback(async (userId: string | undefined) => {
    if (!userId) {
      setProfile(null);
      setSettings(null);
      return;
    }
    setLoading(true);
    const [profileRes, settingsRes] = await Promise.all([
      supabase.from('profiles').select('id, username, display_name').eq('id', userId).maybeSingle(),
      supabase.from('user_settings').select('sharing_enabled, radius_m').eq('user_id', userId).maybeSingle(),
    ]);
    setProfile(profileRes.data);
    setSettings(settingsRes.data);
    setLoading(false);
    if (settingsRes.data) {
      syncTracking(settingsRes.data.sharing_enabled).catch((e) => console.warn('tracking sync failed', e));
      if (settingsRes.data.sharing_enabled) registerForPushNotifications();
    }
  }, []);

  useEffect(() => {
    supabase.auth.getSession().then(async ({ data }) => {
      setSession(data.session);
      await loadUserData(data.session?.user.id);
      setLoading(false); // Also covers the signed-out case.
    });
    const { data: sub } = supabase.auth.onAuthStateChange((event, next) => {
      setSession(next);
      if (event === 'SIGNED_IN') setLoading(true);
      // Defer: awaiting other Supabase calls inside this callback can deadlock.
      if (event === 'SIGNED_IN' || event === 'SIGNED_OUT') setTimeout(() => loadUserData(next?.user.id), 0);
    });
    return () => sub.subscription.unsubscribe();
  }, [loadUserData]);

  const refresh = useCallback(() => loadUserData(session?.user.id), [loadUserData, session]);

  const updateSettings = useCallback(
    async (patch: Partial<Settings>) => {
      if (!session) return;
      const { data, error } = await supabase
        .from('user_settings')
        .update(patch)
        .eq('user_id', session.user.id)
        .select('sharing_enabled, radius_m')
        .single();
      if (error) throw error;
      setSettings(data);
      if ('sharing_enabled' in patch) await syncTracking(data.sharing_enabled);
    },
    [session],
  );

  const signOut = useCallback(async () => {
    // Signing out stops sharing so friends don't see a location that's no
    // longer being updated.
    await stopBackgroundUpdates();
    if (session) {
      await supabase.from('user_settings').update({ sharing_enabled: false }).eq('user_id', session.user.id);
      await clearPushToken();
    }
    await supabase.auth.signOut();
  }, [session]);

  return (
    <SessionContext.Provider
      value={{ loading, session, profile, settings, refresh, updateSettings, signOut }}>
      {children}
    </SessionContext.Provider>
  );
}
