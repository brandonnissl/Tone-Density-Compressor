import Slider from '@react-native-community/slider';
import { useEffect, useState } from 'react';
import { Alert, Linking, ScrollView, StyleSheet, Switch, Text, View } from 'react-native';

import { formatDistance, radiusToSlider, sliderToRadius } from '@/lib/format';
import { getPermissionLevel, requestPermissions, type PermissionLevel } from '@/lib/location';
import { registerForPushNotifications } from '@/lib/notifications';
import { useSession } from '@/lib/session';
import { Button, Card, Label, useTheme } from '@/lib/ui';

export default function SettingsScreen() {
  const t = useTheme();
  const { profile, settings, updateSettings, signOut } = useSession();
  const [permission, setPermission] = useState<PermissionLevel | null>(null);
  const [radius, setRadius] = useState(settings?.radius_m ?? 1000);
  const [busy, setBusy] = useState(false);

  useEffect(() => {
    getPermissionLevel().then(setPermission);
  }, [settings?.sharing_enabled]);

  useEffect(() => {
    if (settings) setRadius(settings.radius_m);
  }, [settings]);

  async function toggleSharing(on: boolean) {
    setBusy(true);
    try {
      if (on) {
        const level = await requestPermissions();
        setPermission(level);
        if (level === 'none') {
          Alert.alert(
            'Location permission needed',
            'Allow location access in your phone’s settings to share with friends.',
            [{ text: 'Cancel', style: 'cancel' }, { text: 'Open Settings', onPress: () => Linking.openSettings() }],
          );
          return;
        }
        await registerForPushNotifications();
      }
      await updateSettings({ sharing_enabled: on });
    } catch (e) {
      Alert.alert('Could not update sharing', e instanceof Error ? e.message : String(e));
    } finally {
      setBusy(false);
    }
  }

  async function saveRadius(value: number) {
    try {
      await updateSettings({ radius_m: sliderToRadius(value) });
    } catch (e) {
      Alert.alert('Could not save distance', e instanceof Error ? e.message : String(e));
    }
  }

  return (
    <ScrollView contentContainerStyle={s.container}>
      <Card>
        <View style={s.row}>
          <View style={s.flex}>
            <Text style={[s.title, { color: t.text }]}>Share my location</Text>
            <Label muted>Only with friends you switch on in the Friends tab, and only if they share back.</Label>
          </View>
          <Switch
            accessibilityLabel="Share my location"
            value={settings?.sharing_enabled ?? false}
            onValueChange={toggleSharing}
            disabled={busy}
          />
        </View>
        {settings?.sharing_enabled && permission === 'foreground' && (
          <View style={s.notice}>
            <Label muted>
              Location is only updated while the app is open. To get alerts when a friend comes near, set location
              access to “Always”.
            </Label>
            <Button title="Open phone settings" variant="secondary" onPress={() => Linking.openSettings()} />
          </View>
        )}
      </Card>

      <Card>
        <Text style={[s.title, { color: t.text }]}>Nearby distance</Text>
        <Label muted>Show friends and alert me when they are within this distance.</Label>
        <Text style={[s.radius, { color: t.accent }]}>{formatDistance(radius)}</Text>
        <Slider
          accessibilityLabel="Nearby distance"
          minimumValue={0}
          maximumValue={1}
          value={radiusToSlider(settings?.radius_m ?? 1000)}
          onValueChange={(v) => setRadius(sliderToRadius(v))}
          onSlidingComplete={saveRadius}
          minimumTrackTintColor={t.accent}
        />
        <View style={s.row}>
          <Label muted>100 m</Label>
          <View style={s.flex} />
          <Label muted>50 km</Label>
        </View>
      </Card>

      <Card>
        <Text style={[s.title, { color: t.text }]}>Privacy</Text>
        <Label muted>
          Your location is rounded to about 100 m on your phone before it is sent. Friends never see where you
          are, only roughly how far away you are. Turning sharing off deletes your stored location right away.
        </Label>
      </Card>

      <Card>
        <Label muted>
          Signed in as {profile?.display_name} (@{profile?.username})
        </Label>
        <Button title="Sign out" variant="danger" onPress={signOut} />
        <Label muted>Signing out also turns sharing off.</Label>
      </Card>
    </ScrollView>
  );
}

const s = StyleSheet.create({
  container: { padding: 16, gap: 12 },
  flex: { flex: 1 },
  row: { flexDirection: 'row', alignItems: 'center', gap: 12 },
  title: { fontSize: 17, fontWeight: '600' },
  radius: { fontSize: 28, fontWeight: '700' },
  notice: { gap: 8, marginTop: 4 },
});
