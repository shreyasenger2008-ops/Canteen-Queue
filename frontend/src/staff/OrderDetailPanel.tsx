import React from 'react';
import type { Order, Counter } from '../types';
import {
  Play, CheckCircle2, PackageCheck, XCircle,
  CreditCard, CheckCheck, Loader2, AlertCircle,
  Clock, MapPin, User, ShoppingBag,
} from 'lucide-react';
import { getItemImage } from '../student/MenuSection';

interface Props {
  order: Order | null;
  counters: Counter[];
  onAction: (action: 'accept' | 'start' | 'ready' | 'collect' | 'cancel' | 'pay', order: Order) => void;
  actionLoading: string | null;
  actionMsg: { ok: boolean; text: string } | null;
  onClearMsg: () => void;
}

type ActionDef = {
  id:    'accept' | 'start' | 'ready' | 'collect' | 'cancel' | 'pay';
  label: string;
  icon:  React.FC<any>;
  primary?: boolean;
  danger?: boolean;
  cond:  (o: Order) => boolean;
};

const ACTIONS: ActionDef[] = [
  {
    id: 'accept', label: 'Accept order', icon: CheckCheck,
    primary: true,
    cond: o => o.status === 'QUEUED',
  },
  {
    id: 'start', label: 'Start cooking', icon: Play,
    primary: true,
    cond: o => o.status === 'QUEUED' || o.status === 'PREPARING',
  },
  {
    id: 'ready', label: 'Mark ready', icon: CheckCircle2,
    primary: true,
    cond: o => o.status === 'PREPARING',
  },
  {
    id: 'collect', label: 'Collect order', icon: PackageCheck,
    primary: true,
    cond: o => o.status === 'READY',
  },
  {
    id: 'pay', label: 'Mark paid', icon: CreditCard,
    primary: true,
    cond: o => o.payment_status !== 'PAID' && o.status !== 'CANCELLED',
  },
  {
    id: 'cancel', label: 'Cancel order', icon: XCircle,
    danger: true,
    cond: o => o.status === 'QUEUED' || o.status === 'PREPARING',
  },
];

const STATUS_LABELS: Record<string, { label: string; bg: string; ink: string }> = {
  QUEUED:    { label: 'Queued in line',   bg: 'rgba(240,120,32,0.18)',  ink: '#F07820'  },
  PREPARING: { label: 'Cooking',          bg: 'rgba(245,158,11,0.18)', ink: '#F59E0B'  },
  READY:     { label: 'Ready for pickup', bg: 'rgba(34,197,94,0.18)',  ink: '#22C55E'  },
  COLLECTED: { label: 'Collected',        bg: 'rgba(99,179,237,0.15)', ink: '#63B3ED'  },
  CANCELLED: { label: 'Cancelled',        bg: 'rgba(239,68,68,0.18)',  ink: '#EF4444'  },
};

export const OrderDetailPanel: React.FC<Props> = ({
  order, counters, onAction, actionLoading, actionMsg, onClearMsg,
}) => {
  const counter = order ? counters.find(c => c.id === order.assigned_counter_id) : null;

  if (!order) {
    return (
      <div
        className="flex flex-col items-center justify-center gap-4 px-6 py-16 text-center"
        style={{
          background: 'var(--surface)',
          border: '1px solid var(--border)',
          borderRadius: '16px',
          minHeight: 500,
        }}
      >
        <div
          className="w-16 h-16 rounded-2xl flex items-center justify-center"
          style={{ background: 'var(--raised)' }}
        >
          <ShoppingBag className="w-8 h-8" style={{ color: 'var(--faint)' }} />
        </div>
        <p className="text-sm font-semibold" style={{ color: 'var(--muted)' }}>Select an order</p>
        <p className="text-xs max-w-xs" style={{ color: 'var(--faint)' }}>
          Click any order on the left to view its details and take action on it.
        </p>
      </div>
    );
  }

  const sl = STATUS_LABELS[order.status] || { label: order.status, bg: 'var(--raised)', ink: 'var(--muted)' };
  const available = ACTIONS.filter(a => a.cond(order));

  return (
    <div
      className="flex flex-col overflow-hidden"
      style={{
        background: 'var(--surface)',
        border: '1px solid var(--border)',
        borderRadius: '16px',
        minHeight: 500,
      }}
    >
      {/* Header */}
      <div className="px-5 py-4 border-b flex items-center justify-between" style={{ borderColor: 'var(--border)' }}>
        <div>
          <div className="flex items-center gap-2 flex-wrap">
            <h2 className="text-xl font-bold" style={{ color: 'var(--text)' }}>
              Order #{order.id}
            </h2>
            <span
              className="text-xs font-medium px-2.5 py-0.5 rounded-full"
              style={{ background: sl.bg, color: sl.ink }}
            >
              {sl.label}
            </span>
            {order.payment_status === 'PAID' && (
              <span className="text-xs font-medium px-2.5 py-0.5 rounded-full"
                style={{ background: 'rgba(34,197,94,0.18)', color: '#22C55E' }}>
                Paid
              </span>
            )}
          </div>
          <p className="text-xs mt-0.5" style={{ color: 'var(--muted)' }}>
            {new Date(order.order_timestamp * 1000).toLocaleString()}
          </p>
        </div>
      </div>

      <div className="flex-1 overflow-y-auto px-5 py-4 space-y-5">
        {/* Action message */}
        {actionMsg && (
          <div
            className="flex items-center gap-2 px-3 py-2.5 rounded-xl text-sm"
            style={{
              background: actionMsg.ok ? 'rgba(34,197,94,0.15)' : 'rgba(239,68,68,0.15)',
              color: actionMsg.ok ? '#22C55E' : '#EF4444',
            }}
          >
            {actionMsg.ok ? <CheckCircle2 className="w-4 h-4 shrink-0" /> : <AlertCircle className="w-4 h-4 shrink-0" />}
            <span className="flex-1">{actionMsg.text}</span>
            <button onClick={onClearMsg} className="text-xs underline opacity-60 cursor-pointer">dismiss</button>
          </div>
        )}

        {/* Info grid */}
        <div className="grid grid-cols-2 gap-2.5">
          {[
            { icon: User,        label: 'Student',   value: `${order.student_name} (${order.student_id})` },
            { icon: MapPin,      label: 'Counter',   value: counter ? counter.name : `Counter #${order.assigned_counter_id}` },
            { icon: Clock,       label: 'Prep time', value: `~${order.total_prep_time_minutes} min` },
            { icon: ShoppingBag, label: 'Amount',    value: `₹${order.total_amount}` },
          ].map(({ icon: Icon, label, value }) => (
            <div key={label} className="p-3 rounded-xl" style={{ background: 'var(--raised)' }}>
              <div className="flex items-center gap-1.5 mb-1">
                <Icon className="w-3.5 h-3.5" style={{ color: 'var(--primary)' }} />
                <span className="text-[10px] font-medium" style={{ color: 'var(--muted)' }}>{label}</span>
              </div>
              <p className="text-sm font-medium truncate" style={{ color: 'var(--text)' }}>{value}</p>
            </div>
          ))}
        </div>

        {/* Items */}
        <div>
          <p className="text-xs font-medium mb-2" style={{ color: 'var(--muted)' }}>Order items</p>
          <div className="space-y-1.5">
            {order.items.map(item => {
              const imgUrl = getItemImage(item.item_name);
              return (
                <div
                  key={item.item_id}
                  className="flex items-center justify-between px-3 py-2.5 rounded-xl text-sm"
                  style={{ background: 'var(--raised)' }}
                >
                  <div className="flex items-center gap-2.5">
                    {imgUrl && (
                      <img
                        src={imgUrl}
                        alt={item.item_name}
                        className="w-8 h-8 rounded-lg object-cover shrink-0 border"
                        style={{ borderColor: 'var(--border)' }}
                      />
                    )}
                    <span className="font-medium" style={{ color: 'var(--text)' }}>
                      {item.item_name}
                      <span className="ml-2 text-xs font-normal" style={{ color: 'var(--muted)' }}>× {item.quantity}</span>
                    </span>
                  </div>
                  <span className="font-semibold" style={{ color: 'var(--text)' }}>₹{item.price * item.quantity}</span>
                </div>
              );
            })}
            <div
              className="flex items-center justify-between px-3 py-2.5 rounded-xl font-semibold text-sm border-t"
              style={{ borderColor: 'var(--border)', color: 'var(--text)' }}
            >
              <span>Total</span>
              <span style={{ color: 'var(--primary)' }}>₹{order.total_amount}</span>
            </div>
          </div>
        </div>

        {/* Action buttons */}
        {available.length > 0 && (
          <div>
            <p className="text-xs font-medium mb-2" style={{ color: 'var(--muted)' }}>Actions</p>
            <div className="grid grid-cols-2 gap-2">
              {available.map(a => {
                const Icon = a.icon;
                const isLoading = actionLoading === a.id;
                let bg = 'var(--raised)', color = 'var(--text)', border = 'var(--border)';
                if (a.primary) { bg = 'var(--primary)'; color = '#fff'; border = 'var(--primary)'; }
                if (a.danger)  { bg = 'rgba(239,68,68,0.15)'; color = '#EF4444'; border = 'transparent'; }
                return (
                  <button
                    key={a.id}
                    onClick={() => onAction(a.id, order)}
                    disabled={!!actionLoading}
                    className="flex items-center justify-center gap-2 px-4 py-2.5 font-medium text-sm border transition disabled:opacity-50 cursor-pointer"
                    style={{ background: bg, color, borderColor: border, borderRadius: '10px' }}
                  >
                    {isLoading
                      ? <Loader2 className="w-4 h-4 animate-spin" />
                      : <Icon className="w-4 h-4" />
                    }
                    {a.label}
                  </button>
                );
              })}
            </div>
          </div>
        )}

        {/* Queue position */}
        {order.queue_position != null && order.queue_position > 0 && !['COLLECTED','CANCELLED'].includes(order.status) && (
          <div
            className="flex items-center gap-3 px-4 py-3 rounded-xl border-l-4"
            style={{ background: 'var(--raised)', borderLeftColor: 'var(--primary)' }}
          >
            <span className="font-bold text-2xl" style={{ color: 'var(--primary)' }}>#{order.queue_position}</span>
            <div>
              <p className="text-sm font-medium" style={{ color: 'var(--text)' }}>Position in queue</p>
              <p className="text-xs" style={{ color: 'var(--muted)' }}>at Counter #{order.assigned_counter_id}</p>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};
