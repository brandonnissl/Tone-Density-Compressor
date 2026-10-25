import Constants from 'expo-constants';
import * as Device from 'expo-device';
import * as Notifications from 'expo-notifications';
import { Platform } from 'react-native';

import { supabase } from './supabase';

Notifications.setNotificationHandler({
  handleNotification: async () => ({
    shouldShowBanner: true,
    shouldShowList: true,
    shouldPlaySound: true,
    shouldSetBadge: false,
  }),
});

// Asks for notification permission and saves this device's Expo push token so
// the server can send "friend nearby" alerts. Safe to call repeatedly.
export async function registerForPushNotifications() {
  if (Platform.OS === 'android') {
    await Notifications.setNotificationChannelAsync('nearby', {
      name: 'Friends nearby',
      importance: Notifications.AndroidImportance.HIGH,
    });
  }

  if (!Device.isDevice) return; // Simulators can't receive push.

  let { status } = await Notifications.getPermissionsAsync();
  if (status !== 'granted') {
    ({ status } = await Notifications.requestPermissionsAsync());
  }
  if (status !== 'granted') return;

  const projectId = Constants.expoConfig?.extra?.eas?.projectId;
  if (!projectId) {
    console.warn('No EAS projectId in app.json; push alerts are disabled. Run `eas init`.');
    return;
  }

  try {
    const { data: token } = await Notifications.getExpoPushTokenAsync({ projectId });
    await supabase.rpc('set_push_token', { p_token: token });
  } catch (e) {
    console.warn('Could not get a push token', e);
  }
}

export async function clearPushToken() {
  await supabase.rpc('set_push_token', { p_token: '' });
}
