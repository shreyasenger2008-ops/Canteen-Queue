import React from 'react';
import type { HealthStatus } from '../types';
import type { StudentView } from './StudentApp';
import { ShoppingCart } from 'lucide-react';

interface Props {
  view: StudentView;
  onNav: (v: StudentView) => void;
  cartCount: number;
  onOpenCart: () => void;
  health: HealthStatus | null;
  onGoStaff: () => void;
}

export const StudentHeader: React.FC<Props> = ({
  view, onNav, cartCount, onOpenCart, health, onGoStaff,
}) => {
  const online = health?.status === 'healthy';

  const navLink = (v: StudentView, label: string) => (
    <button
      key={v}
      onClick={() => onNav(v)}
      className="text-sm cursor-pointer transition-colors"
      style={{
        color: view === v ? 'var(--text)' : 'var(--muted)',
        fontWeight: view === v ? 600 : 400,
        background: 'none',
        border: 'none',
        borderBottom: `2px solid ${view === v ? 'var(--primary)' : 'transparent'}`,
        paddingBottom: '2px',
        paddingLeft: 0,
        paddingRight: 0,
      }}
    >
      {label}
    </button>
  );

  return (
    <header
      className="sticky top-0 z-50 border-b"
      style={{ background: 'var(--surface)', borderColor: 'var(--border)' }}
    >
      <div className="max-w-6xl mx-auto px-4 sm:px-6 h-16 flex items-center justify-between gap-4">
        {/* Brand */}
        <button
          onClick={() => onNav('home')}
          className="flex items-center gap-0 select-none cursor-pointer shrink-0"
          style={{ background: 'none', border: 'none', padding: 0 }}
        >
          <span
            className="font-display text-xl"
            style={{ color: 'var(--text)' }}
          >
            Canteen
          </span>
          <span
            className="font-display text-xl"
            style={{ color: 'var(--primary)' }}
          >
            Dine
          </span>
        </button>

        {/* Nav */}
        <nav className="hidden md:flex items-center gap-6">
          {navLink('home',   'Home')}
          {navLink('menu',   'Menu')}
          {navLink('track',  'Track Order')}
          {navLink('queues', 'Live Queues')}
        </nav>

        {/* Right */}
        <div className="flex items-center gap-3">
          {/* Online indicator */}
          <div className="hidden sm:flex items-center gap-1.5 text-xs" style={{ color: 'var(--muted)' }}>
            <span
              className="w-2 h-2 rounded-full"
              style={{ background: online ? '#22c55e' : '#ef4444' }}
            />
            {online ? 'Open' : 'Offline'}
          </div>

          {/* Staff link */}
          <button
            onClick={onGoStaff}
            className="hidden sm:inline-flex text-xs cursor-pointer"
            style={{ color: 'var(--muted)', background: 'none', border: 'none', padding: 0 }}
          >
            Staff →
          </button>

          {/* Cart */}
          <button
            onClick={onOpenCart}
            className="relative flex items-center gap-2 px-4 py-2 text-sm font-semibold text-white cursor-pointer transition"
            style={{ background: 'var(--primary)', borderRadius: '999px' }}
          >
            <ShoppingCart className="w-4 h-4" />
            <span className="hidden sm:inline">Cart</span>
            {cartCount > 0 && (
              <span
                className="w-5 h-5 rounded-full text-xs font-bold flex items-center justify-center"
                style={{ background: '#fff', color: 'var(--primary)' }}
              >
                {cartCount}
              </span>
            )}
          </button>
        </div>
      </div>

      {/* Mobile nav */}
      <div className="md:hidden border-t flex" style={{ borderColor: 'var(--border)' }}>
        {(['home', 'menu', 'track'] as StudentView[]).map(v => (
          <button
            key={v}
            onClick={() => onNav(v)}
            className="flex-1 py-2 text-xs font-medium capitalize transition cursor-pointer"
            style={{
              color: view === v ? 'var(--primary)' : 'var(--muted)',
              background: 'none',
              border: 'none',
              borderBottom: `2px solid ${view === v ? 'var(--primary)' : 'transparent'}`,
            }}
          >
            {v === 'track' ? 'Track' : v.charAt(0).toUpperCase() + v.slice(1)}
          </button>
        ))}
      </div>
    </header>
  );
};
