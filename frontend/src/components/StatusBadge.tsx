import React from 'react';
import type { OrderStatus, PaymentStatus } from '../types';
import { Clock, ChefHat, CheckCircle2, PackageCheck, XCircle, CreditCard, Check } from 'lucide-react';

interface StatusProps {
  status: OrderStatus;
  size?: 'sm' | 'md' | 'lg';
}

export const StatusBadge: React.FC<StatusProps> = ({ status, size = 'md' }) => {
  const configs: Record<OrderStatus, { label: string; bg: string; text: string; icon: React.ReactNode }> = {
    QUEUED:    { label: 'Queued',    bg: 'var(--pill-warning-bg)',  text: 'var(--pill-warning-ink)',  icon: <Clock className="w-3 h-3" /> },
    PREPARING: { label: 'Cooking',   bg: 'var(--pill-warning-bg)',  text: 'var(--pill-warning-ink)',  icon: <ChefHat className="w-3 h-3" /> },
    READY:     { label: 'Ready',     bg: 'var(--pill-positive-bg)', text: 'var(--pill-positive-ink)', icon: <CheckCircle2 className="w-3 h-3" /> },
    COLLECTED: { label: 'Collected', bg: 'var(--pill-positive-bg)', text: 'var(--pill-positive-ink)', icon: <PackageCheck className="w-3 h-3" /> },
    CANCELLED: { label: 'Cancelled', bg: 'var(--pill-negative-bg)', text: 'var(--pill-negative-ink)', icon: <XCircle className="w-3 h-3" /> },
  };

  const config = configs[status] || configs.QUEUED;
  const sizeClass = { sm: 'text-[11px] px-2 py-0.5 gap-1', md: 'text-xs px-2.5 py-0.5 gap-1', lg: 'text-xs px-3 py-1 gap-1.5' }[size];

  return (
    <span className={`inline-flex items-center font-medium ${sizeClass}`}
      style={{ background: config.bg, color: config.text, borderRadius: '6px' }}>
      {config.icon}
      <span>{config.label}</span>
    </span>
  );
};

interface PaymentProps {
  status?: PaymentStatus | string;
  size?: 'sm' | 'md';
}

export const PaymentBadge: React.FC<PaymentProps> = ({ status = 'PENDING', size = 'sm' }) => {
  const isPaid = status === 'PAID';
  const sizeClass = size === 'sm' ? 'text-[11px] px-2 py-0.5 gap-1' : 'text-xs px-2.5 py-0.5 gap-1';

  if (isPaid) {
    return (
      <span className={`inline-flex items-center font-medium ${sizeClass}`}
        style={{ background: 'var(--pill-positive-bg)', color: 'var(--pill-positive-ink)', borderRadius: '6px' }}>
        <Check className="w-3 h-3" /><span>Paid</span>
      </span>
    );
  }

  return (
    <span className={`inline-flex items-center font-medium ${sizeClass}`}
      style={{ background: 'var(--raised)', color: 'var(--muted)', borderRadius: '6px', border: '1px solid var(--border)' }}>
      <CreditCard className="w-3 h-3" /><span>Pay at counter</span>
    </span>
  );
};
