export function formatDistance(meters: number) {
  if (meters < 1000) return `${meters} m`;
  return `${(meters / 1000).toFixed(meters < 10_000 ? 1 : 0)} km`;
}

export function formatAgo(iso: string) {
  const minutes = Math.max(0, Math.round((Date.now() - new Date(iso).getTime()) / 60_000));
  if (minutes < 1) return 'just now';
  if (minutes < 60) return `${minutes} min ago`;
  return `${Math.round(minutes / 60)} h ago`;
}

// The radius slider runs on a log scale so short and long distances are both
// easy to pick: 0 → 100 m, 1 → 50 km.
const MIN_RADIUS = 100;
const MAX_RADIUS = 50_000;

export function sliderToRadius(t: number) {
  const raw = MIN_RADIUS * Math.pow(MAX_RADIUS / MIN_RADIUS, t);
  const step = raw < 1000 ? 50 : raw < 10_000 ? 250 : 1000;
  return Math.min(MAX_RADIUS, Math.max(MIN_RADIUS, Math.round(raw / step) * step));
}

export function radiusToSlider(meters: number) {
  return Math.log(meters / MIN_RADIUS) / Math.log(MAX_RADIUS / MIN_RADIUS);
}
