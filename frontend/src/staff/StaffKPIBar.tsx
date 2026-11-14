import React from 'react';
import type { AnalyticsData, Counter } from '../types';
import { ShoppingCart, ChefHat, CheckCircle2, XCircle } from 'lucide-react';

interface Props {
  analytics: AnalyticsData | null;
  counters: Counter[];
}

export const StaffKPIBar: React.FC<Props> = ({ analytics, counters }) => {
  const totalServed = counters.reduce((s, c) => s + c.total_orders_served, 0);

  const kpis = [
    {
      label: 'Pending Orders',
      value: analytics?.currently_pending_orders ?? 0,
      icon: ShoppingCart,
      iconColor: 'var(--primary)',
    },
    {
      label: 'Active Workload',
      value: `${analytics?.total_active_workload_minutes ?? 0}m`,
      icon: ChefHat,
      iconColor: '#F59E0B',
    },
    {
      label: 'Orders Served',
      value: totalServed,
      icon: CheckCircle2,
      iconColor: 'var(--positive)',
    },
    {
      label: 'Active Counters',
      value: counters.filter(c => c.is_available).length,
      icon: XCircle,
      iconColor: 'var(--negative)',
    },
  ];

  return (
    <div className="grid grid-cols-2 sm:grid-cols-4 gap-4">
      {kpis.map(k => {
        const Icon = k.icon;
        return (
          <div
            key={k.label}
            className="flex items-center justify-between p-5"
            style={{
              background: 'var(--surface)',
              border: '1px solid var(--border)',
              borderRadius: '16px',
            }}
          >
            <div>
              <p className="text-xs font-medium mb-1" style={{ color: 'var(--muted)' }}>{k.label}</p>
              <p className="text-4xl font-bold num leading-none" style={{ color: 'var(--text)' }}>{k.value}</p>
            </div>
            <Icon className="w-8 h-8 shrink-0" style={{ color: k.iconColor }} />
          </div>
        );
      })}
    </div>
  );
};
