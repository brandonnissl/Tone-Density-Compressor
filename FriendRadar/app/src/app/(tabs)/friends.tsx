import { useFocusEffect } from 'expo-router';
import { useCallback, useState } from 'react';
import { Alert, RefreshControl, SectionList, StyleSheet, Switch, Text, View } from 'react-native';

import { supabase, type Friend } from '@/lib/supabase';
import { Button, Card, Input, Label, useTheme } from '@/lib/ui';

export default function Friends() {
  const t = useTheme();
  const [friends, setFriends] = useState<Friend[]>([]);
  const [username, setUsername] = useState('');
  const [busy, setBusy] = useState(false);
  const [refreshing, setRefreshing] = useState(false);

  const load = useCallback(async () => {
    const { data, error } = await supabase.rpc('my_friends');
    if (!error) setFriends((data ?? []) as Friend[]);
  }, []);

  useFocusEffect(
    useCallback(() => {
      load();
    }, [load]),
  );

  async function run(fn: () => PromiseLike<{ error: { message: string } | null }>) {
    const { error } = await fn();
    if (error) Alert.alert('Something went wrong', error.message);
    await load();
  }

  async function addFriend() {
    setBusy(true);
    const { data, error } = await supabase.rpc('send_friend_request', { p_username: username.trim() });
    setBusy(false);
    if (error) {
      Alert.alert('Could not add friend', error.message);
      return;
    }
    setUsername('');
    if (data === 'accepted') Alert.alert('You’re now friends', 'They had already sent you a request.');
    await load();
  }

  function confirmRemove(f: Friend) {
    Alert.alert(`Remove ${f.display_name}?`, 'You’ll stop seeing each other.', [
      { text: 'Cancel', style: 'cancel' },
      {
        text: 'Remove',
        style: 'destructive',
        onPress: () => run(() => supabase.rpc('remove_friend', { p_friend: f.friend_id })),
      },
    ]);
  }

  const sections = [
    { title: 'Requests', data: friends.filter((f) => f.status === 'incoming') },
    { title: 'Friends', data: friends.filter((f) => f.status === 'accepted') },
    { title: 'Sent', data: friends.filter((f) => f.status === 'outgoing') },
  ].filter((section) => section.data.length > 0);

  return (
    <SectionList
      contentContainerStyle={s.list}
      sections={sections}
      keyExtractor={(f) => f.friend_id}
      refreshControl={
        <RefreshControl
          refreshing={refreshing}
          onRefresh={async () => {
            setRefreshing(true);
            await load();
            setRefreshing(false);
          }}
        />
      }
      ListHeaderComponent={
        <Card style={s.header}>
          <Label>Add a friend by username</Label>
          <Input
            placeholder="username"
            autoCapitalize="none"
            autoCorrect={false}
            value={username}
            onChangeText={(v) => setUsername(v.toLowerCase())}
            onSubmitEditing={addFriend}
          />
          <Button title="Send request" onPress={addFriend} busy={busy} disabled={username.trim().length < 3} />
        </Card>
      }
      ListEmptyComponent={<Label muted>No friends yet. Add someone above to get started.</Label>}
      renderSectionHeader={({ section }) => (
        <Text style={[s.sectionTitle, { color: t.muted }]}>{section.title}</Text>
      )}
      renderItem={({ item: f }) => (
        <Card>
          <View style={s.row}>
            <View style={s.flex}>
              <Text style={[s.name, { color: t.text }]}>{f.display_name}</Text>
              <Text style={{ color: t.muted }}>@{f.username}</Text>
            </View>
            {f.status === 'accepted' && (
              <View style={s.toggle}>
                <Text style={{ color: t.muted, fontSize: 13 }}>Share with</Text>
                <Switch
                  accessibilityLabel={`Share my location with ${f.display_name}`}
                  value={f.i_share}
                  onValueChange={(v) =>
                    run(() => supabase.rpc('set_share_with_friend', { p_friend: f.friend_id, p_share: v }))
                  }
                />
              </View>
            )}
          </View>

          {f.status === 'accepted' && (
            <Label muted>
              {f.i_share && f.they_share
                ? 'You can see each other when nearby.'
                : f.i_share
                  ? 'Waiting for them to share with you. Neither of you is visible until they do.'
                  : f.they_share
                    ? 'They’re sharing with you. Turn on sharing to see each other.'
                    : 'Not sharing with each other.'}
            </Label>
          )}

          {f.status === 'incoming' && (
            <View style={s.actions}>
              <View style={s.flex}>
                <Button
                  title="Accept"
                  onPress={() =>
                    run(() => supabase.rpc('respond_friend_request', { p_from: f.friend_id, p_accept: true }))
                  }
                />
              </View>
              <View style={s.flex}>
                <Button
                  title="Decline"
                  variant="secondary"
                  onPress={() =>
                    run(() => supabase.rpc('respond_friend_request', { p_from: f.friend_id, p_accept: false }))
                  }
                />
              </View>
            </View>
          )}

          {f.status === 'outgoing' && (
            <Button
              title="Cancel request"
              variant="secondary"
              onPress={() => run(() => supabase.rpc('remove_friend', { p_friend: f.friend_id }))}
            />
          )}
          {f.status === 'accepted' && <Button title="Remove friend" variant="danger" onPress={() => confirmRemove(f)} />}
        </Card>
      )}
    />
  );
}

const s = StyleSheet.create({
  flex: { flex: 1 },
  list: { padding: 16, gap: 12 },
  header: { marginBottom: 8 },
  sectionTitle: { fontSize: 13, fontWeight: '600', textTransform: 'uppercase', marginTop: 8 },
  row: { flexDirection: 'row', alignItems: 'center' },
  toggle: { alignItems: 'center', gap: 2 },
  name: { fontSize: 17, fontWeight: '600' },
  actions: { flexDirection: 'row', gap: 8 },
});
