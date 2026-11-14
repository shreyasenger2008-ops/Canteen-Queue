import React from 'react';
import { Zap, Brain, Radio } from 'lucide-react';

interface Props {
  onOrder: () => void;
  onTrack: () => void;
}

export const HeroSection: React.FC<Props> = ({ onOrder, onTrack }) => {
  return (
    <section style={{ background: 'var(--ground)' }}>
      <div className="max-w-6xl mx-auto px-6 py-16 sm:py-24">
        <div className="flex flex-col-reverse sm:flex-row items-center gap-12 sm:gap-16">

          {/* Left — Text */}
          <div className="flex-1 flex flex-col gap-5">
            {/* Pill label */}
            <span
              className="self-start text-xs font-semibold px-4 py-1.5 tracking-wide"
              style={{ background: 'var(--primary)', color: '#fff', borderRadius: '999px' }}
            >
              Fresh Daily · Fast Queue · ML Powered
            </span>

            {/* Headline */}
            <h1
              className="font-display leading-tight"
              style={{ color: 'var(--text)', fontSize: 'clamp(36px, 5vw, 54px)' }}
            >
              Taste the Tradition,<br />
              Served Fresh Daily
            </h1>

            {/* Subtitle */}
            <p className="text-base sm:text-lg leading-relaxed" style={{ color: 'var(--muted)', maxWidth: 480 }}>
              Order from campus canteen — your counter assigned in seconds by our AI queue engine.
            </p>

            {/* CTAs */}
            <div className="flex flex-wrap gap-3 mt-1">
              <button
                onClick={onOrder}
                className="btn-orange px-7 py-3 text-sm font-semibold"
              >
                Order Now
              </button>
              <button
                onClick={onTrack}
                className="px-7 py-3 text-sm font-semibold border-2 cursor-pointer transition"
                style={{
                  borderColor: 'var(--primary)',
                  color: 'var(--primary)',
                  background: 'transparent',
                  borderRadius: '10px',
                }}
              >
                Track Order
              </button>
            </div>

            {/* Stat chips */}
            <div className="flex flex-wrap gap-3 mt-2">
              {[
                { icon: <Zap className="w-3.5 h-3.5" />, label: 'FIFO Queue' },
                { icon: <Brain className="w-3.5 h-3.5" />, label: 'ML Prediction' },
                { icon: <Radio className="w-3.5 h-3.5" />, label: 'Live Tracking' },
              ].map(({ icon, label }) => (
                <span
                  key={label}
                  className="inline-flex items-center gap-1.5 px-3 py-1.5 text-xs font-medium border"
                  style={{
                    background: 'var(--raised)',
                    borderColor: 'var(--border)',
                    color: 'var(--muted)',
                    borderRadius: '8px',
                  }}
                >
                  {icon}
                  {label}
                </span>
              ))}
            </div>
          </div>

          {/* Right — Food image */}
          <div className="relative shrink-0 flex items-center justify-center">
            <img
              src="/images/menu/poha.jpg"
              alt="Fresh canteen food"
              style={{
                width: 300,
                height: 300,
                borderRadius: '50%',
                objectFit: 'cover',
                border: '8px solid white',
                boxShadow: '0 8px 40px rgba(240,120,32,0.25)',
              }}
            />

            {/* Floating badge bottom-left */}
            <div
              className="absolute -bottom-3 -left-4 flex items-center gap-2 px-4 py-2 shadow-lg"
              style={{
                background: 'var(--surface)',
                borderRadius: '12px',
                boxShadow: '0 4px 20px rgba(0,0,0,0.10)',
                border: '1px solid var(--border)',
              }}
            >
              <span
                className="w-8 h-8 flex items-center justify-center rounded-full"
                style={{ background: 'var(--raised)' }}
              >
                <Zap className="w-4 h-4" style={{ color: 'var(--primary)' }} />
              </span>
              <div>
                <p className="text-xs font-semibold" style={{ color: 'var(--text)' }}>Fast Delivery</p>
                <p className="text-[11px]" style={{ color: 'var(--muted)' }}>~5 min avg wait</p>
              </div>
            </div>
          </div>

        </div>
      </div>
    </section>
  );
};
