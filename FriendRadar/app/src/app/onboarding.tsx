import { Redirect } from 'expo-router';
import { useState } from 'react';
import { Alert, StyleSheet, Text, View } from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';

import { useSession } from '@/lib/session';
import { supabase } from '@/lib/supabase';
import { Button, Input, Label, useTheme } from '@/lib/ui';

const USERNAME = /^[a-z0-9_]{3,20}$/;

export default function Onboarding() {
  const t = useTheme();
  const { session, profile, refresh } = useSession();
  const [username, setUsername] = useState('');
  const [displayName, setDisplayName] = useState('');
  const [busy, setBusy] = useState(false);

  if (!session) return <Redirect href="/sign-in" />;
  if (profile) return <Redirect href="/" />;

  const valid = USERNAME.test(username) && displayName.trim().length > 0;

  async function save() {
    setBusy(true);
    const { error } = await supabase
      .from('profiles')
      .insert({ id: session!.user.id, username, display_name: displayName.trim() });
    setBusy(false);
    if (error) {
      const taken = error.code === '23505';
      Alert.alert(taken ? 'Username taken' : 'Could not save', taken ? 'Try another one.' : error.message);
      return;
    }
    await refresh();
  }

  return (
    <SafeAreaView style={[s.flex, { backgroundColor: t.bg }]}>
      <View style={s.container}>
        <Text style={[s.title, { color: t.text }]}>Set up your profile</Text>
        <Label muted>Friends add you by username. Your name is what they see.</Label>
        <Input
          placeholder="username"
          autoCapitalize="none"
          autoCorrect={false}
          value={username}
          onChangeText={(v) => setUsername(v.toLowerCase())}
        />
        <Label muted>3–20 characters: letters, numbers, underscores.</Label>
        <Input placeholder="Your name" value={displayName} onChangeText={setDisplayName} maxLength={40} />
        <Button title="Continue" onPress={save} busy={busy} disabled={!valid} />
      </View>
    </SafeAreaView>
  );
}

const s = StyleSheet.create({
  flex: { flex: 1 },
  container: { flex: 1, padding: 24, gap: 12, justifyContent: 'center' },
  title: { fontSize: 28, fontWeight: '700' },
});
