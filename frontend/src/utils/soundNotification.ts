import { useEffect, useRef } from 'react';

/**
 * Synthesises a pleasant 2-tone chime using the Web Audio API.
 * C5 (523 Hz) → E5 (659 Hz), each ~0.3 s, sine wave with gain fade-out.
 * Silent-fails on browsers without AudioContext support.
 */
export function playReadyChime(): void {
  try {
    const AudioCtx =
      window.AudioContext ||
      (window as unknown as { webkitAudioContext: typeof AudioContext })
        .webkitAudioContext;
    if (!AudioCtx) return;

    const ctx = new AudioCtx();

    const tones = [
      { freq: 523.25, start: 0 },   // C5
      { freq: 659.25, start: 0.32 }, // E5
    ];

    tones.forEach(({ freq, start }) => {
      const osc = ctx.createOscillator();
      const gain = ctx.createGain();

      osc.type = 'sine';
      osc.frequency.setValueAtTime(freq, ctx.currentTime + start);

      // Gentle attack + fade-out envelope
      gain.gain.setValueAtTime(0, ctx.currentTime + start);
      gain.gain.linearRampToValueAtTime(0.35, ctx.currentTime + start + 0.02);
      gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + start + 0.3);

      osc.connect(gain);
      gain.connect(ctx.destination);

      osc.start(ctx.currentTime + start);
      osc.stop(ctx.currentTime + start + 0.32);
    });

    // Close context after the chime finishes to free resources
    setTimeout(() => {
      ctx.close().catch(() => {});
    }, 900);
  } catch {
    // Silently ignore – browser may not support AudioContext
  }
}

/**
 * React hook that watches for an order status transition → "READY".
 * On that transition it plays the chime and (if permission is granted)
 * shows a browser Notification.
 *
 * @returns `{ requestPermission }` – call this to ask the user to allow notifications.
 */
export function useOrderReadyNotification(currentStatus: string | undefined): {
  requestPermission: () => Promise<void>;
} {
  const prevStatusRef = useRef<string | undefined>(undefined);
  const mountedRef = useRef(false);

  useEffect(() => {
    // Skip the very first render (initial mount)
    if (!mountedRef.current) {
      mountedRef.current = true;
      prevStatusRef.current = currentStatus;
      return;
    }

    const prev = prevStatusRef.current;
    prevStatusRef.current = currentStatus;

    // Fire only when transitioning FROM a non-READY status TO "READY"
    if (currentStatus === 'READY' && prev !== 'READY') {
      playReadyChime();

      if (
        typeof Notification !== 'undefined' &&
        Notification.permission === 'granted'
      ) {
        try {
          new Notification('🍽️ Order Ready!', {
            body: 'Your order is ready for collection at the counter.',
            icon: '/favicon.ico',
          });
        } catch {
          // Notification API may still be blocked in some contexts
        }
      }
    }
  }, [currentStatus]);

  const requestPermission = async (): Promise<void> => {
    if (typeof Notification === 'undefined') return;
    if (Notification.permission === 'default') {
      await Notification.requestPermission();
    }
  };

  return { requestPermission };
}
