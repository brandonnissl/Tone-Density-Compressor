import { Redirect } from 'expo-router';
import { useState } from 'react';
import { Alert, KeyboardAvoidingView, Platform, StyleSheet, Text, View } from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';

import { useSession } from '@/lib/session';
import { supabase } from '@/lib/supabase';
import { Button, Input, Label, useTheme } from '@/lib/ui';

export default function SignIn() {
  const t = useTheme();
  const { session } = useSession();
  const [email, setEmail] = useState('');
  const [code, setCode] = useState('');
  const [sent, setSent] = useState(false);
  const [busy, setBusy] = useState(false);

  if (session) return <Redirect href="/" />;

  async function sendCode() {
    setBusy(true);
    const { error } = await supabase.auth.signInWithOtp({ email: email.trim() });
    setBusy(false);
    if (error) Alert.alert('Could not send code', error.message);
    else setSent(true);
  }

  async function verify() {
    setBusy(true);
    const { error } = await supabase.auth.verifyOtp({ email: email.trim(), token: code.trim(), type: 'email' });
    setBusy(false);
    if (error) Alert.alert('That code didn’t work', error.message);
  }

  return (
    <SafeAreaView style={[s.flex, { backgroundColor: t.bg }]}>
      <KeyboardAvoidingView behavior={Platform.OS === 'ios' ? 'padding' : undefined} style={s.container}>
        <View style={s.header}>
          <Text style={[s.title, { color: t.text }]}>Friend Radar</Text>
          <Label muted>See which friends are nearby. Only friends you pick, only when you both opt in.</Label>
        </View>
        {!sent ? (
          <>
            <Input
              placeholder="you@example.com"
              autoCapitalize="none"
              autoComplete="email"
              keyboardType="email-address"
              value={email}
              onChangeText={setEmail}
            />
            <Button title="Email me a code" onPress={sendCode} busy={busy} disabled={!email.includes('@')} />
          </>
        ) : (
          <>
            <Label muted>Enter the 6-digit code sent to {email.trim()}.</Label>
            <Input
              placeholder="123456"
              keyboardType="number-pad"
              autoComplete="one-time-code"
              maxLength={6}
              value={code}
              onChangeText={setCode}
            />
            <Button title="Sign in" onPress={verify} busy={busy} disabled={code.trim().length < 6} />
            <Button title="Use a different email" variant="secondary" onPress={() => setSent(false)} />
          </>
        )}
      </KeyboardAvoidingView>
    </SafeAreaView>
  );
}

const s = StyleSheet.create({
  flex: { flex: 1 },
  container: { flex: 1, padding: 24, gap: 12, justifyContent: 'center' },
  header: { gap: 8, marginBottom: 12 },
  title: { fontSize: 32, fontWeight: '700' },
});
