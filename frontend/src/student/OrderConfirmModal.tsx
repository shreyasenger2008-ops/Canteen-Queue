import React from 'react';
import type { Order, Counter } from '../types';
import { CheckCircle2, MapPin, Clock, X, ArrowRight, CreditCard } from 'lucide-react';

interface Props {
  order: Order;
  counters: Counter[];
  onClose: () => void;
  onTrack: (id: number) => void;
}

export const OrderConfirmModal: React.FC<Props> = ({ order, counters, onClose, onTrack }) => {
  const counter = counters.find(c => c.id === order.assigned_counter_id);

  return (
    <div className="fixed inset-0 z-[60] flex items-center justify-center p-4 animate-fade-in">
      <div className="absolute inset-0 bg-black/40" onClick={onClose} />
      <div
        className="relative w-full max-w-sm animate-fade-up overflow-hidden"
        style={{ background: 'var(--surface)', borderRadius: '24px', boxShadow: '0 8px 40px rgba(0,0,0,.14)' }}
      >
        {/* Close */}
        <button
          onClick={onClose}
          className="absolute top-3 right-3 w-8 h-8 flex items-center justify-center cursor-pointer z-10"
          style={{ background: 'var(--raised)', borderRadius: '8px', border: 'none' }}
        >
          <X className="w-4 h-4" style={{ color: 'var(--muted)' }} />
        </button>

        {/* Top section */}
        <div className="px-6 pt-8 pb-5 flex flex-col items-center text-center gap-3">
          <div
            className="w-16 h-16 rounded-full flex items-center justify-center"
            style={{ background: 'var(--pill-positive-bg)' }}
          >
            <CheckCircle2 className="w-9 h-9" style={{ color: 'var(--pill-positive-ink)' }} />
          </div>
          <h3 className="font-display text-2xl" style={{ color: 'var(--text)' }}>Order Confirmed!</h3>
          <p className="text-sm" style={{ color: 'var(--muted)' }}>Queued in line — your counter is ready</p>
        </div>

        {/* Details */}
        <div className="px-6 pb-6 space-y-3">
          {/* Order ID row */}
          <div
            className="flex items-center justify-between px-4 py-3 border"
            style={{ background: 'var(--raised)', borderColor: 'var(--border)', borderRadius: '12px' }}
          >
            <span className="text-sm" style={{ color: 'var(--muted)' }}>Order ID</span>
            <span className="font-bold text-base num" style={{ color: 'var(--text)' }}>#{order.id}</span>
          </div>

          <div className="grid grid-cols-2 gap-3">
            <div className="p-3 border flex flex-col gap-1" style={{ background: 'var(--raised)', borderColor: 'var(--border)', borderRadius: '12px' }}>
              <MapPin className="w-4 h-4" style={{ color: 'var(--muted)' }} />
              <span className="text-xs" style={{ color: 'var(--muted)' }}>Counter</span>
              <span className="font-semibold text-sm" style={{ color: 'var(--text)' }}>
                {counter ? counter.name : `Counter #${order.assigned_counter_id}`}
              </span>
            </div>
            <div className="p-3 border flex flex-col gap-1" style={{ background: 'var(--raised)', borderColor: 'var(--border)', borderRadius: '12px' }}>
              <Clock className="w-4 h-4" style={{ color: 'var(--primary)' }} />
              <span className="text-xs" style={{ color: 'var(--muted)' }}>Est. prep</span>
              <span className="font-semibold text-sm num" style={{ color: 'var(--text)' }}>~{order.total_prep_time_minutes} min</span>
            </div>
          </div>

          <div className="flex items-center justify-between px-4 py-3 border" style={{ background: 'var(--raised)', borderColor: 'var(--border)', borderRadius: '12px' }}>
            <span className="text-sm" style={{ color: 'var(--muted)' }}>Total</span>
            <span className="font-bold text-lg num" style={{ color: 'var(--primary)' }}>₹{order.total_amount}</span>
          </div>

          <div className="flex items-center gap-2 px-3 py-2.5 text-xs" style={{ background: 'var(--pill-warning-bg)', color: 'var(--pill-warning-ink)', borderRadius: '8px' }}>
            <CreditCard className="w-3.5 h-3.5 shrink-0" />
            Pay at the counter when you collect your order
          </div>

          {/* Buttons */}
          <div className="flex gap-2 pt-1">
            <button
              onClick={onClose}
              className="flex-1 py-2.5 font-medium text-sm border cursor-pointer transition"
              style={{ borderColor: 'var(--border)', color: 'var(--muted)', background: 'var(--surface)', borderRadius: '10px' }}
            >
              Back to Menu
            </button>
            <button
              onClick={() => onTrack(order.id)}
              className="flex-1 py-2.5 font-semibold text-sm text-white flex items-center justify-center gap-1.5 cursor-pointer"
              style={{ background: 'var(--primary)', borderRadius: '10px', border: 'none' }}
            >
              Track My Order <ArrowRight className="w-3.5 h-3.5" />
            </button>
          </div>
        </div>
      </div>
    </div>
  );
};
