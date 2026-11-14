import React from 'react';
import type { Order } from '../types';
import { Loader2, ChevronLeft, ChevronRight, ClipboardList } from 'lucide-react';

const STATUS_FILTERS = ['ALL', 'QUEUED', 'PREPARING', 'READY', 'COLLECTED', 'CANCELLED'];

const STATUS_MAP: Record<string, { label: string; bg: string; color: string }> = {
  QUEUED:    { label: 'Queued',    bg: 'rgba(240,120,32,0.18)',  color: '#F07820' },
  PREPARING: { label: 'Cooking',   bg: 'rgba(245,158,11,0.18)', color: '#F59E0B' },
  READY:     { label: 'Ready',     bg: 'rgba(34,197,94,0.18)',  color: '#22C55E' },
  COLLECTED: { label: 'Collected', bg: 'rgba(99,179,237,0.15)', color: '#63B3ED' },
  CANCELLED: { label: 'Cancelled', bg: 'rgba(239,68,68,0.18)',  color: '#EF4444' },
};

interface Props {
  orders: Order[];
  total: number;
  page: number;
  pageSize: number;
  onPageChange: (p: number) => void;
  statusFilter: string;
  onStatusFilter: (f: string) => void;
  selected: Order | null;
  onSelect: (o: Order) => void;
  loading: boolean;
}

export const OrderTicketsPanel: React.FC<Props> = ({
  orders, total, page, pageSize, onPageChange,
  statusFilter, onStatusFilter, selected, onSelect, loading,
}) => {
  const totalPages = Math.ceil(total / pageSize);

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
      {/* Panel header */}
      <div className="px-5 py-4 border-b" style={{ borderColor: 'var(--border)' }}>
        <h2 className="font-bold text-xl" style={{ color: 'var(--text)' }}>Order Queue</h2>
        <p className="text-xs mt-0.5" style={{ color: 'var(--muted)' }}>
          {total} order{total !== 1 ? 's' : ''} · Auto-refreshing every 4s
        </p>
      </div>

      {/* Status filter chips */}
      <div className="px-4 py-3 flex gap-2 overflow-x-auto scrollbar-none border-b" style={{ borderColor: 'var(--border)' }}>
        {STATUS_FILTERS.map(f => {
          const active = f === statusFilter;
          return (
            <button
              key={f}
              onClick={() => onStatusFilter(f)}
              className="shrink-0 px-3 py-1.5 rounded-xl text-xs font-bold transition cursor-pointer"
              style={{
                background: active ? 'var(--primary)' : 'var(--raised)',
                color:      active ? '#fff' : 'var(--muted)',
                border:     `1px solid ${active ? 'var(--primary)' : 'var(--border)'}`,
              }}
            >
              {f === 'ALL' ? 'All Orders' : f.charAt(0) + f.slice(1).toLowerCase()}
            </button>
          );
        })}
      </div>

      {/* Order list */}
      {loading ? (
        <div className="flex-1 flex items-center justify-center">
          <Loader2 className="w-8 h-8 animate-spin" style={{ color: 'var(--primary)' }} />
        </div>
      ) : orders.length === 0 ? (
        <div className="flex-1 flex flex-col items-center justify-center gap-3 py-12 px-6 text-center">
          <div
            className="w-14 h-14 rounded-2xl flex items-center justify-center"
            style={{ background: 'var(--raised)' }}
          >
            <ClipboardList className="w-7 h-7" style={{ color: 'var(--faint)' }} />
          </div>
          <p className="font-semibold text-sm" style={{ color: 'var(--muted)' }}>No orders</p>
          <p className="text-xs" style={{ color: 'var(--faint)' }}>
            {statusFilter === 'ALL' ? 'Waiting for the first order…' : `No ${statusFilter.toLowerCase()} orders.`}
          </p>
        </div>
      ) : (
        <div className="flex-1 overflow-y-auto divide-y" style={{ borderColor: 'var(--border)' }}>
          {orders.map(order => {
            const s = STATUS_MAP[order.status] || { label: order.status, bg: 'var(--raised)', color: 'var(--muted)' };
            const isSelected = selected?.id === order.id;
            return (
              <button
                key={order.id}
                onClick={() => onSelect(order)}
                className="w-full text-left px-5 py-4 flex items-center gap-4 cursor-pointer transition"
                style={{
                  background: isSelected ? 'var(--active)' : 'transparent',
                  borderLeft: isSelected ? '3px solid var(--primary)' : '3px solid transparent',
                }}
              >
                {/* Order number badge */}
                <div
                  className="shrink-0 w-12 h-12 rounded-2xl flex items-center justify-center font-black text-sm"
                  style={{
                    background: isSelected ? 'var(--primary)' : 'var(--raised)',
                    color: isSelected ? '#fff' : 'var(--text)',
                  }}
                >
                  #{order.id}
                </div>

                {/* Details */}
                <div className="flex-1 min-w-0">
                  <div className="flex items-center gap-2 flex-wrap">
                    <span className="font-bold text-sm truncate" style={{ color: 'var(--text)' }}>
                      {order.student_name}
                    </span>
                    <span
                      className="text-[10px] font-bold px-2 py-0.5 rounded-full"
                      style={{ background: s.bg, color: s.color }}
                    >
                      {s.label}
                    </span>
                    {order.payment_status === 'PAID' && (
                      <span
                        className="text-[10px] font-bold px-2 py-0.5 rounded-full"
                        style={{ background: 'rgba(34,197,94,0.18)', color: '#22C55E' }}
                      >
                        Paid
                      </span>
                    )}
                  </div>
                  <p className="text-xs mt-0.5" style={{ color: 'var(--muted)' }}>
                    {order.items.length} item{order.items.length !== 1 ? 's' : ''} · ₹{order.total_amount} ·
                    Counter #{order.assigned_counter_id}
                  </p>
                </div>

                {/* Time */}
                <div className="text-xs text-right shrink-0" style={{ color: 'var(--muted)' }}>
                  {new Date(order.order_timestamp * 1000).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}
                </div>
              </button>
            );
          })}
        </div>
      )}

      {/* Pagination */}
      {totalPages > 1 && (
        <div
          className="px-5 py-3 border-t flex items-center justify-between"
          style={{ borderColor: 'var(--border)' }}
        >
          <span className="text-xs" style={{ color: 'var(--muted)' }}>
            Page {page + 1} of {totalPages}
          </span>
          <div className="flex gap-2">
            <button
              onClick={() => onPageChange(page - 1)}
              disabled={page === 0}
              title="Previous Page"
              className="w-8 h-8 rounded-xl border flex items-center justify-center disabled:opacity-40 transition cursor-pointer"
              style={{ borderColor: 'var(--border)', color: 'var(--text)', background: 'var(--raised)' }}
            >
              <ChevronLeft className="w-4 h-4" />
            </button>
            <button
              onClick={() => onPageChange(page + 1)}
              disabled={page >= totalPages - 1}
              title="Next Page"
              className="w-8 h-8 rounded-xl border flex items-center justify-center disabled:opacity-40 transition cursor-pointer"
              style={{ borderColor: 'var(--border)', color: 'var(--text)', background: 'var(--raised)' }}
            >
              <ChevronRight className="w-4 h-4" />
            </button>
          </div>
        </div>
      )}
    </div>
  );
};
