import { Link, useFocusEffect } from 'expo-router';
import { useCallback, useState } from 'react';
import { FlatList, RefreshControl, StyleSheet, Text, View } from 'react-native';

import { formatAgo, formatDistance } from '@/lib/format';
import { reportCurrentLocation } from '@/lib/location';
import { useSession } from '@/lib/session';
import { supabase, type NearbyFriend } from '@/lib/supabase';
import { Card, Label, useTheme } from '@/lib/ui';

const POLL_MS = 30_000;

export default function Nearby() {
  const t = useTheme();
  const { settings } = useSession();
  const [friends, setFriends] = useState<NearbyFriend[]>([]);
  const [refreshing, setRefreshing] = useState(false);

  const load = useCallback(async (sendLocation: boolean) => {
    if (sendLocation) await reportCurrentLocation().catch(() => {});
    const { data, error } = await supabase.rpc('nearby_friends');
    if (!error) setFriends((data ?? []) as NearbyFriend[]);
  }, []);

  // Refresh while the screen is visible.
  useFocusEffect(
    useCallback(() => {
      if (!settings?.sharing_enabled) return;
      load(true);
      const id = setInterval(() => load(false), POLL_MS);
      return () => clearInterval(id);
    }, [load, settings?.sharing_enabled]),
  );

  async function onRefresh() {
    setRefreshing(true);
    await load(true);
    setRefreshing(false);
  }

  if (!settings?.sharing_enabled) {
    return (
      <View style={s.empty}>
        <Card>
          <Label>Location sharing is off.</Label>
          <Label muted>
            Sharing works both ways: you’ll only see friends who can also see you. Turn it on in{' '}
            <Link href="/settings" style={{ color: t.accent }}>
              Settings
            </Link>
            .
          </Label>
        </Card>
      </View>
    );
  }

  return (
    <FlatList
      contentContainerStyle={s.list}
      data={friends}
      keyExtractor={(f) => f.friend_id}
      refreshControl={<RefreshControl refreshing={refreshing} onRefresh={onRefresh} />}
      ListHeaderComponent={
        <Label muted>Friends within {formatDistance(settings.radius_m)} who share with you.</Label>
      }
      ListEmptyComponent={
        <Card>
          <Label>Nobody nearby right now.</Label>
          <Label muted>You’ll get a notification when a friend comes within range.</Label>
        </Card>
      }
      renderItem={({ item }) => (
        <Card style={s.row}>
          <View style={s.flex}>
            <Text style={[s.name, { color: t.text }]}>{item.display_name}</Text>
            <Text style={{ color: t.muted }}>@{item.username} · {formatAgo(item.updated_at)}</Text>
          </View>
          <Text style={[s.distance, { color: t.accent }]}>~{formatDistance(item.approx_distance_m)}</Text>
        </Card>
      )}
    />
  );
}

const s = StyleSheet.create({
  flex: { flex: 1 },
  empty: { padding: 16 },
  list: { padding: 16, gap: 12 },
  row: { flexDirection: 'row', alignItems: 'center' },
  name: { fontSize: 17, fontWeight: '600' },
  distance: { fontSize: 17, fontWeight: '600' },
});
