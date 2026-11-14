import React from 'react';
import { RefreshCw, LogOut, ChevronLeft, Radio, ChefHat } from 'lucide-react';

interface Props {
  onLogout:    () => void;
  onGoStudent: () => void;
  health:      { status: string } | null;
  onRefresh:   () => void;
  isRefreshing: boolean;
}

export const StaffHeader: React.FC<Props> = ({
  onLogout, onGoStudent, health, onRefresh, isRefreshing,
}) => {
  const online = health?.status === 'healthy';

  return (
    <header
      className="sticky top-0 z-50 border-b"
      style={{ background: 'var(--surface)', borderColor: 'var(--border)' }}
    >
      <div className="max-w-7xl mx-auto px-4 sm:px-6 h-14 flex items-center justify-between gap-4">
        {/* Left */}
        <div className="flex items-center gap-4">
          <button
            onClick={onGoStudent}
            className="flex items-center gap-1 text-xs font-medium transition cursor-pointer"
            style={{ color: 'var(--muted)', background: 'none', border: 'none', padding: 0 }}
          >
            <ChevronLeft className="w-4 h-4" />
            <span className="hidden sm:inline">Student app</span>
          </button>

          <div className="w-px h-5" style={{ background: 'var(--border)' }} />

          {/* Wordmark */}
          <div className="flex items-center gap-2">
            <span
              className="w-7 h-7 rounded flex items-center justify-center text-xs font-bold text-white select-none"
              style={{ background: 'var(--primary)' }}
              aria-hidden="true"
            >
              <ChefHat className="w-4 h-4" />
            </span>
            <div>
              <div className="text-sm font-semibold leading-none" style={{ color: 'var(--text)' }}>
                Kitchen Ops
              </div>
              <div className="text-[10px] mt-0.5" style={{ color: 'var(--muted)' }}>
                Canteen staff dashboard
              </div>
            </div>
          </div>
        </div>

        {/* Right */}
        <div className="flex items-center gap-2">
          {/* System status pill */}
          <div
            className="hidden sm:flex items-center gap-1.5 text-xs font-medium px-2.5 py-1 rounded-full border"
            style={
              online
                ? { background: 'var(--pill-positive-bg)', color: 'var(--pill-positive-ink)', borderColor: 'transparent' }
                : { background: 'var(--pill-negative-bg)', color: 'var(--pill-negative-ink)', borderColor: 'transparent' }
            }
          >
            <Radio className="w-3 h-3" style={{ opacity: online ? 1 : 0.7 }} />
            {online ? 'System online' : 'Offline'}
          </div>

          <button
            onClick={onRefresh}
            className="w-8 h-8 rounded flex items-center justify-center transition cursor-pointer"
            style={{ color: 'var(--muted)', border: '1px solid var(--border)', background: 'var(--raised)' }}
            title="Refresh data"
          >
            <RefreshCw className={`w-4 h-4 ${isRefreshing ? 'animate-spin' : ''}`} />
          </button>

          <button
            onClick={onLogout}
            className="flex items-center gap-1.5 text-xs font-medium px-3 py-1.5 rounded border transition cursor-pointer"
            style={{ borderColor: 'var(--primary)', color: 'var(--primary)', background: 'transparent' }}
          >
            <LogOut className="w-3.5 h-3.5" />
            <span className="hidden sm:inline">Sign out</span>
          </button>
        </div>
      </div>
    </header>
  );
};
