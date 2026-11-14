import React, { useState, useEffect } from 'react';
import { RefreshCw, Clock, CheckCircle2, AlertCircle, Layers } from 'lucide-react';
import type { Counter, AnalyticsData, HealthStatus } from '../types';
import { getCounters, getAnalytics } from '../services/api';

interface Props {
  health: HealthStatus | null;
}

export const QueueDashboard: React.FC<Props> = ({ health }) => {
  const [counters,     setCounters]     = useState<Counter[]>([]);
  const [analytics,    setAnalytics]    = useState<AnalyticsData | null>(null);
  const [loading,      setLoading]      = useState<boolean>(true);
  const [error,        setError]        = useState<string | null>(null);
  const [autoRefresh,  setAutoRefresh]  = useState<boolean>(true);
  const [lastUpdated,  setLastUpdated]  = useState<Date>(new Date());

  const fetchData = async () => {
    try {
      setError(null);
      const [countersData, analyticsData] = await Promise.all([getCounters(), getAnalytics()]);
      setCounters(countersData);
      setAnalytics(analyticsData);
      setLastUpdated(new Date());
    } catch (err: any) {
      setError(err.message || 'Error communicating with C++ backend');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => { fetchData(); }, []);

  useEffect(() => {
    if (!autoRefresh) return;
    const interval = setInterval(fetchData, 4000);
    return () => clearInterval(interval);
  }, [autoRefresh]);

  const getWorkloadLabel = (w: number) => {
    if (w === 0)  return { label: 'Idle',        pillBg: 'var(--raised)',            pillInk: 'var(--muted)'             };
    if (w <= 10)  return { label: 'Low',         pillBg: 'var(--pill-positive-bg)',  pillInk: 'var(--pill-positive-ink)' };
    if (w <= 20)  return { label: 'Moderate',    pillBg: 'var(--pill-warning-bg)',   pillInk: 'var(--pill-warning-ink)'  };
    return               { label: 'High demand', pillBg: 'var(--pill-negative-bg)',  pillInk: 'var(--pill-negative-ink)' };
  };

  return (
    <div className="space-y-6 pb-16">
      {/* Header */}
      <div className="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3">
        <div>
          <div className="flex items-center gap-2">
            <h1 className="text-2xl font-bold" style={{ color: 'var(--text)' }}>Live Queue Status</h1>
            <span className="flex items-center gap-1 text-xs font-medium px-2 py-0.5"
              style={{ background: 'var(--pill-positive-bg)', color: 'var(--pill-positive-ink)', borderRadius: '4px' }}>
              <span className="w-1.5 h-1.5 rounded-full animate-pulse-dot" style={{ background: 'var(--positive)' }} />
              Live
            </span>
          </div>
          <p className="text-sm mt-1" style={{ color: 'var(--muted)' }}>
            Real-time counter queues — C++ Array &amp; FIFO engine with ML load balancing.
          </p>
        </div>
        <div className="flex items-center gap-2">
          <button onClick={() => setAutoRefresh(!autoRefresh)} className="flex items-center gap-1.5 px-3 py-1.5 text-xs font-medium border cursor-pointer transition"
            style={{ background: autoRefresh ? 'var(--raised)' : 'var(--surface)', color: autoRefresh ? 'var(--text)' : 'var(--muted)', borderColor: 'var(--border)', borderRadius: '6px' }}>
            <span className="w-1.5 h-1.5 rounded-full" style={{ background: autoRefresh ? 'var(--positive)' : 'var(--faint)' }} />
            Auto refresh (4s)
          </button>
          <button onClick={fetchData} disabled={loading} className="flex items-center gap-1.5 px-3 py-1.5 text-xs font-medium border cursor-pointer transition"
            style={{ background: 'var(--surface)', borderColor: 'var(--border)', color: 'var(--muted)', borderRadius: '6px' }}>
            <RefreshCw className={`w-3.5 h-3.5 ${loading ? 'animate-spin' : ''}`} />
            Refresh
          </button>
        </div>
      </div>

      {/* Error */}
      {error && (
        <div className="p-3.5 border flex items-center gap-2 text-sm"
          style={{ background: 'rgba(239,68,68,0.15)', borderColor: 'rgba(239,68,68,0.3)', color: '#EF4444', borderRadius: '8px' }}>
          <AlertCircle className="w-4 h-4 shrink-0" /><span>{error}</span>
          <button onClick={fetchData} className="ml-auto text-xs font-medium underline cursor-pointer">Retry</button>
        </div>
      )}

      {/* KPI divided strip */}
      {analytics && (
        <div className="border flex flex-col sm:flex-row" style={{ background: 'var(--surface)', borderColor: 'var(--border)', borderRadius: '12px' }}>
          {[
            { label: 'Total orders',    value: analytics.total_orders_history_count,         accent: false, small: false },
            { label: 'In queue now',    value: analytics.currently_pending_orders,            accent: true,  small: false },
            { label: 'Active workload', value: `${analytics.total_active_workload_minutes}m`, accent: false, small: false },
            { label: 'ML engine',       value: analytics.ml_model_active ? 'Random Forest' : 'Baseline', accent: false, small: true },
          ].map((kpi, i, arr) => (
            <div key={kpi.label} className="flex-1 px-5 py-4"
              style={{ borderRight: i < arr.length - 1 ? '1px solid var(--border)' : 'none' }}>
              <p className="text-xs font-medium" style={{ color: 'var(--muted)' }}>{kpi.label}</p>
              <p className={`mt-1 font-semibold num ${kpi.small ? 'text-sm' : 'text-2xl'}`}
                style={{ color: kpi.accent ? 'var(--primary)' : 'var(--text)' }}>{kpi.value}</p>
            </div>
          ))}
        </div>
      )}

      {/* Loading */}
      {loading && counters.length === 0 && (
        <div className="py-16 flex items-center justify-center gap-2" style={{ color: 'var(--muted)' }}>
          <RefreshCw className="w-4 h-4 animate-spin" /><span className="text-sm">Loading live queue data…</span>
        </div>
      )}

      {/* Counter cards */}
      {counters.length > 0 && (
        <div>
          <h2 className="text-base font-semibold mb-3" style={{ color: 'var(--text)' }}>Counters ({counters.length})</h2>
          <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
            {counters.map(counter => {
              const wl = getWorkloadLabel(counter.current_workload_minutes);
              const maxWorkload = Math.max(...counters.map(c => c.current_workload_minutes), 1);
              const barWidth = counter.current_workload_minutes > 0
                ? Math.min(100, (counter.current_workload_minutes / maxWorkload) * 88) : 0;

              return (
                <div key={counter.id} className="border p-5" style={{ background: 'var(--surface)', borderColor: 'var(--border)', borderRadius: '16px' }}>
                  <div className="flex items-start justify-between mb-3">
                    <div>
                      <div className="font-semibold text-sm" style={{ color: 'var(--text)' }}>{counter.name}</div>
                      <div className="text-xs mt-0.5" style={{ color: 'var(--faint)' }}>{counter.is_available ? 'Available' : 'Unavailable'}</div>
                    </div>
                    <span className="text-[11px] font-medium px-2 py-0.5" style={{ background: wl.pillBg, color: wl.pillInk, borderRadius: '4px' }}>{wl.label}</span>
                  </div>

                  {/* Workload bar */}
                  <div className="mb-4">
                    <div className="h-1.5 rounded-full" style={{ background: 'var(--raised)' }}>
                      <div className="h-1.5 rounded-full transition-all duration-500" style={{ width: `${barWidth}%`, background: 'var(--primary)' }} />
                    </div>
                    <div className="flex justify-between text-xs mt-1" style={{ color: 'var(--faint)' }}>
                      <span>{counter.current_workload_minutes}m workload</span>
                      <span>{counter.pending_orders_count} in queue</span>
                    </div>
                  </div>

                  {/* Stats */}
                  <div className="grid grid-cols-3 gap-2">
                    {[
                      { label: 'Queue',  value: counter.pending_orders_count,            icon: Layers       },
                      { label: 'Served', value: counter.total_orders_served,     icon: CheckCircle2 },
                      { label: 'Wait',   value: `${counter.current_workload_minutes}m`, icon: Clock },
                    ].map(stat => {
                      const Icon = stat.icon;
                      return (
                        <div key={stat.label} className="p-2.5" style={{ background: 'var(--raised)', borderRadius: '8px' }}>
                          <Icon className="w-3.5 h-3.5 mb-1" style={{ color: 'var(--faint)' }} />
                          <p className="text-base font-semibold num" style={{ color: 'var(--text)' }}>{stat.value}</p>
                          <p className="text-[10px]" style={{ color: 'var(--faint)' }}>{stat.label}</p>
                        </div>
                      );
                    })}
                  </div>
                </div>
              );
            })}
          </div>
        </div>
      )}

      {/* Health */}
      {health && (
        <div className="border p-4 flex flex-wrap gap-4 text-xs" style={{ background: 'var(--surface)', borderColor: 'var(--border)', borderRadius: '12px', color: 'var(--muted)' }}>
          <span>Status: <strong style={{ color: 'var(--text)' }}>{health.status}</strong></span>
          <span>Queue engine: <strong style={{ color: 'var(--text)' }}>{(health as any).queue_engine || 'C++ FIFO'}</strong></span>
          <span>DB: <strong style={{ color: 'var(--text)' }}>{(health as any).database || 'SQLite'}</strong></span>
          <span className="ml-auto" style={{ color: 'var(--faint)' }}>Updated {lastUpdated.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}</span>
        </div>
      )}
    </div>
  );
};
