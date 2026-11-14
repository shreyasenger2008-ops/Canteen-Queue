import React, { useState, useEffect } from 'react';
import type { Order, Counter } from '../types';
import { getOrderById, cancelOrder } from '../services/api';
import { getRecentOrders, updateRecentOrderStatus } from '../utils/storage';
import { Search, Loader2, AlertCircle, Clock, MapPin, User, CheckCircle2, ChefHat, PackageCheck, XCircle, CreditCard } from 'lucide-react';

interface Props {
  initialId?: number | null;
  counters: Counter[];
}

const STATUS_STEPS = [
  { key: 'QUEUED',    icon: Clock,        label: 'Queued'   },
  { key: 'PREPARING', icon: ChefHat,      label: 'Cooking'  },
  { key: 'READY',     icon: PackageCheck, label: 'Ready'    },
  { key: 'COLLECTED', icon: CheckCircle2, label: 'Collected'},
];

const STATUS_IDX: Record<string, number> = {
  QUEUED: 0, PREPARING: 1, READY: 2, COLLECTED: 3, CANCELLED: -1,
};

export const TrackSection: React.FC<Props> = ({ initialId, counters }) => {
  const [searchId,    setSearchId]    = useState(initialId ? String(initialId) : '');
  const [order,       setOrder]       = useState<Order | null>(null);
  const [loading,     setLoading]     = useState(false);
  const [cancelling,  setCancelling]  = useState(false);
  const [error,       setError]       = useState<string | null>(null);
  const recents = getRecentOrders();

  useEffect(() => { if (initialId) fetchOrder(initialId); }, [initialId]);

  useEffect(() => {
    if (!order || order.status === 'COLLECTED' || order.status === 'CANCELLED') return;
    const t = setInterval(async () => {
      try {
        const fresh = await getOrderById(order.id);
        setOrder(fresh);
        updateRecentOrderStatus(fresh.id, fresh.status);
      } catch {}
    }, 4000);
    return () => clearInterval(t);
  }, [order?.id, order?.status]);

  const fetchOrder = async (id?: number) => {
    const target = id ?? Number(searchId.trim());
    if (!target || isNaN(target)) { setError('Enter a valid Order ID.'); return; }
    setLoading(true); setError(null);
    try {
      const o = await getOrderById(target);
      setOrder(o);
      updateRecentOrderStatus(o.id, o.status);
    } catch (e: any) {
      setError(e.message || `Order #${target} not found.`);
      setOrder(null);
    } finally {
      setLoading(false);
    }
  };

  const doCancel = async () => {
    if (!order) return;
    setCancelling(true);
    try { const o = await cancelOrder(order.id); setOrder(o); }
    catch (e: any) { setError(e.message); }
    finally { setCancelling(false); }
  };

  const stepIdx    = order ? (STATUS_IDX[order.status] ?? 0) : -1;
  const counter    = order ? counters.find(c => c.id === order.assigned_counter_id) : null;
  const isCancelled = order?.status === 'CANCELLED';

  return (
    <section className="max-w-2xl mx-auto px-4 sm:px-6 py-10 space-y-6">
      {/* Header */}
      <div>
        <h1 className="font-display text-3xl" style={{ color: 'var(--text)' }}>Track Your Order</h1>
        <p className="text-sm mt-1" style={{ color: 'var(--muted)' }}>Enter your order ID to see real-time status.</p>
      </div>

      {/* Search */}
      <form onSubmit={e => { e.preventDefault(); fetchOrder(); }} className="flex gap-2">
        <div className="relative flex-1">
          <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-3.5 h-3.5" style={{ color: 'var(--faint)' }} />
          <input
            type="number"
            placeholder="Order ID (e.g. 101)"
            value={searchId}
            onChange={e => setSearchId(e.target.value)}
            className="w-full pl-9 pr-3 text-sm border outline-none transition"
            style={{
              background: 'var(--surface)',
              borderColor: 'var(--border)',
              color: 'var(--text)',
              borderRadius: '10px',
              height: '44px',
            }}
          />
        </div>
        <button
          type="submit"
          disabled={loading}
          className="px-6 text-sm font-semibold text-white disabled:opacity-60 cursor-pointer"
          style={{ background: 'var(--primary)', borderRadius: '10px', height: '44px', border: 'none' }}
        >
          {loading ? <Loader2 className="w-4 h-4 animate-spin" /> : 'Track'}
        </button>
      </form>

      {/* Recents */}
      {recents.length > 0 && (
        <div className="flex gap-2 flex-wrap">
          <span className="text-xs self-center" style={{ color: 'var(--muted)' }}>Recent:</span>
          {recents.slice(0, 5).map(r => (
            <button
              key={r.id}
              onClick={() => { setSearchId(String(r.id)); fetchOrder(r.id); }}
              className="text-xs font-mono font-medium px-3 py-1.5 border cursor-pointer transition"
              style={{
                background: order?.id === r.id ? 'var(--primary)' : 'var(--surface)',
                color: order?.id === r.id ? '#fff' : 'var(--text)',
                borderColor: order?.id === r.id ? 'var(--primary)' : 'var(--border)',
                borderRadius: '8px',
              }}
            >
              #{r.id}
            </button>
          ))}
        </div>
      )}

      {/* Error */}
      {error && (
        <div className="flex items-center gap-2 px-4 py-3 text-sm" style={{ background: 'var(--pill-negative-bg)', color: 'var(--pill-negative-ink)', borderRadius: '10px' }}>
          <AlertCircle className="w-4 h-4 shrink-0" />{error}
        </div>
      )}

      {/* Order card */}
      {order && (
        <div
          className="border overflow-hidden animate-fade-up"
          style={{ background: 'var(--surface)', borderColor: 'var(--border)', borderRadius: '16px' }}
        >
          {/* Header */}
          <div
            className="px-5 py-4 border-b flex flex-col sm:flex-row sm:items-center justify-between gap-3"
            style={{ background: 'var(--raised)', borderColor: 'var(--border)' }}
          >
            <div>
              <div className="flex items-center gap-2">
                <h3 className="font-semibold text-base num" style={{ color: 'var(--text)' }}>Order #{order.id}</h3>
                <StatusPill status={order.status} />
              </div>
              <div className="flex flex-wrap gap-3 mt-1.5 text-xs" style={{ color: 'var(--muted)' }}>
                <span className="flex items-center gap-1"><User className="w-3 h-3" />{order.student_name} ({order.student_id})</span>
                {counter && <span className="flex items-center gap-1"><MapPin className="w-3 h-3" />{counter.name}</span>}
                {order.queue_position && order.queue_position > 0 && !['COLLECTED','CANCELLED'].includes(order.status) && (
                  <span className="font-semibold" style={{ color: 'var(--primary)' }}>Position #{order.queue_position} in queue</span>
                )}
              </div>
            </div>
            {!['COLLECTED','CANCELLED'].includes(order.status) && (
              <div className="flex items-center gap-1.5 text-xs font-medium" style={{ color: 'var(--primary)' }}>
                <span className="w-2 h-2 rounded-full animate-pulse-dot" style={{ background: 'var(--primary)' }} />
                Live updates
              </div>
            )}
          </div>

          <div className="px-5 py-5 space-y-5">
            {/* Stepper */}
            {!isCancelled && (
              <div className="relative">
                <div className="absolute top-4 left-4 right-4 h-0.5" style={{ background: 'var(--border)' }} />
                <div
                  className="absolute top-4 left-4 h-0.5 transition-all duration-700"
                  style={{ background: 'var(--primary)', width: stepIdx >= 0 ? `${(stepIdx / (STATUS_STEPS.length - 1)) * 100}%` : '0%' }}
                />
                <div className="relative flex justify-between">
                  {STATUS_STEPS.map((step, i) => {
                    const Icon = step.icon;
                    const done = i <= stepIdx;
                    const current = i === stepIdx;
                    return (
                      <div key={step.key} className="flex flex-col items-center gap-1.5 w-20">
                        <div
                          className={`w-8 h-8 rounded-full flex items-center justify-center z-10 transition-all ${current ? 'scale-110' : ''}`}
                          style={{
                            background: done ? 'var(--primary)' : 'var(--surface)',
                            border: `2px solid ${done ? 'var(--primary)' : 'var(--border)'}`,
                          }}
                        >
                          <Icon className="w-4 h-4" style={{ color: done ? '#fff' : 'var(--faint)' }} />
                        </div>
                        <p className="text-[10px] font-medium text-center leading-tight" style={{ color: done ? 'var(--text)' : 'var(--faint)' }}>{step.label}</p>
                      </div>
                    );
                  })}
                </div>
              </div>
            )}

            {isCancelled && (
              <div className="flex items-center gap-3 p-3" style={{ background: 'var(--pill-negative-bg)', borderRadius: '10px' }}>
                <XCircle className="w-5 h-5 shrink-0" style={{ color: 'var(--pill-negative-ink)' }} />
                <div>
                  <p className="font-medium text-sm" style={{ color: 'var(--pill-negative-ink)' }}>Order cancelled</p>
                  <p className="text-xs" style={{ color: 'var(--pill-negative-ink)', opacity: 0.8 }}>Removed from the queue.</p>
                </div>
              </div>
            )}

            {/* Items */}
            <div className="space-y-1">
              <p className="text-xs font-medium" style={{ color: 'var(--muted)' }}>Items ordered</p>
              {order.items.map(item => (
                <div key={item.item_id} className="flex items-center justify-between text-sm py-1.5 border-b last:border-0" style={{ borderColor: 'var(--border)' }}>
                  <span style={{ color: 'var(--text)' }}>{item.item_name}<span className="ml-2 text-xs" style={{ color: 'var(--faint)' }}>× {item.quantity}</span></span>
                  <span className="num text-sm" style={{ color: 'var(--text)' }}>₹{item.price * item.quantity}</span>
                </div>
              ))}
              <div className="flex justify-between font-semibold pt-2" style={{ color: 'var(--text)' }}>
                <span>Total</span>
                <span className="num" style={{ color: 'var(--primary)' }}>₹{order.total_amount}</span>
              </div>
            </div>

            {/* Payment */}
            {order.payment_status !== 'PAID' && !isCancelled && (
              <div className="flex items-center gap-2 p-3 text-xs" style={{ background: 'var(--pill-warning-bg)', color: 'var(--pill-warning-ink)', borderRadius: '8px' }}>
                <CreditCard className="w-3.5 h-3.5 shrink-0" />Payment pending — pay at the counter when you collect
              </div>
            )}
            {order.payment_status === 'PAID' && (
              <div className="flex items-center gap-2 p-3 text-xs" style={{ background: 'var(--pill-positive-bg)', color: 'var(--pill-positive-ink)', borderRadius: '8px' }}>
                <CheckCircle2 className="w-3.5 h-3.5 shrink-0" />Payment confirmed
              </div>
            )}

            {/* Cancel */}
            {(order.status === 'QUEUED' || order.status === 'PREPARING') && (
              <button
                onClick={doCancel}
                disabled={cancelling}
                className="flex items-center gap-2 px-4 py-2 text-xs font-medium border transition disabled:opacity-60 cursor-pointer"
                style={{ borderColor: 'var(--pill-negative-bg)', color: 'var(--pill-negative-ink)', background: 'var(--pill-negative-bg)', borderRadius: '8px' }}
              >
                {cancelling ? <Loader2 className="w-3.5 h-3.5 animate-spin" /> : <XCircle className="w-3.5 h-3.5" />}
                Cancel order
              </button>
            )}
          </div>
        </div>
      )}
    </section>
  );
};

const StatusPill: React.FC<{ status: string }> = ({ status }) => {
  const map: Record<string, { label: string; bg: string; color: string }> = {
    QUEUED:    { label: 'Queued',    bg: 'var(--pill-warning-bg)',  color: 'var(--pill-warning-ink)'  },
    PREPARING: { label: 'Cooking',   bg: 'var(--raised)',            color: 'var(--muted)'             },
    READY:     { label: 'Ready',     bg: 'var(--pill-positive-bg)', color: 'var(--pill-positive-ink)' },
    COLLECTED: { label: 'Collected', bg: 'var(--raised)',            color: 'var(--muted)'             },
    CANCELLED: { label: 'Cancelled', bg: 'var(--pill-negative-bg)', color: 'var(--pill-negative-ink)' },
  };
  const s = map[status] || { label: status, bg: 'var(--raised)', color: 'var(--muted)' };
  return (
    <span className="text-[11px] font-medium px-2 py-0.5" style={{ background: s.bg, color: s.color, borderRadius: '4px' }}>
      {s.label}
    </span>
  );
};
