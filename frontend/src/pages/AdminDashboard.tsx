import React, { useState, useEffect, useCallback, useRef, useMemo } from 'react';
import { createPortal } from 'react-dom';
import {
  ChefHat, CheckCircle2, PackageCheck, XCircle, Play,
  Search, AlertCircle, Loader2, RefreshCw,
  ChevronLeft, ChevronRight, Filter, CreditCard, Check, Lock, Unlock,
  MoreVertical, Pin, PinOff, Trash2, Eye, X,
} from 'lucide-react';
import type { Order, Counter, AnalyticsData, RecentOrderMeta } from '../types';
import {
  getOrderById, getOrders, startOrder, readyOrder, collectOrder,
  cancelOrder, deleteOrder, payOrder, acceptOrder, getCounters, getAnalytics,
  staffLogin, clearStaffToken, getStaffToken,
} from '../services/api';
import { StatusBadge, PaymentBadge } from '../components/StatusBadge';
import { getRecentOrders, updateRecentOrderStatus } from '../utils/storage';

// ── Reusable action button ────────────────────────────────────────────────────
const ActionBtn: React.FC<{
  label: string; icon: React.ReactNode; disabled: boolean; loading: boolean;
  onClick: () => void; primary?: boolean; danger?: boolean;
}> = ({ label, icon, disabled, loading, onClick, primary, danger }) => {
  let bg = 'var(--raised)', color = 'var(--text)', border = 'var(--border)';
  if (primary && !disabled) { bg = 'var(--primary)'; color = '#fff'; border = 'var(--primary)'; }
  if (danger  && !disabled) { bg = 'rgba(239,68,68,0.15)'; color = '#EF4444'; border = 'transparent'; }
  return (
    <button onClick={onClick} disabled={disabled}
      className="flex items-center gap-1.5 px-3 py-1.5 text-xs font-medium border cursor-pointer disabled:opacity-30 disabled:cursor-not-allowed transition"
      style={{ background: bg, color, borderColor: border, borderRadius: '6px', height: '32px' }}>
      {loading ? <Loader2 className="w-3.5 h-3.5 animate-spin" /> : icon}
      {label}
    </button>
  );
};

// ── Manage Dropdown ───────────────────────────────────────────────────────────
const ManageDropdown: React.FC<{
  orderId: number;
  isPinned: boolean;
  onPin: () => void;
  onDelete: () => void;
  onViewDetails: () => void;
}> = ({ isPinned, onPin, onDelete, onViewDetails }) => {
  const [open, setOpen] = useState(false);
  const [pos, setPos] = useState<{ top: number; right: number; openUp: boolean; isDark: boolean }>({
    top: 0, right: 0, openUp: false, isDark: true,
  });
  const triggerRef = useRef<HTMLButtonElement>(null);
  const menuRef = useRef<HTMLDivElement>(null);

  const updatePosition = useCallback(() => {
    const btn = triggerRef.current;
    if (!btn) return;
    const rect = btn.getBoundingClientRect();
    const menuHeight = 140;
    const openUp = rect.bottom + menuHeight + 8 > window.innerHeight && rect.top > menuHeight;
    setPos({
      top: openUp ? rect.top - 4 : rect.bottom + 4,
      right: Math.max(8, window.innerWidth - rect.right),
      openUp,
      isDark: !!btn.closest('.staff-mode'),
    });
  }, []);

  useEffect(() => {
    if (!open) return;
    updatePosition();
    const handleOutside = (e: MouseEvent) => {
      const target = e.target as Node;
      if (triggerRef.current?.contains(target) || menuRef.current?.contains(target)) return;
      setOpen(false);
    };
    const handleScrollOrResize = () => setOpen(false);
    document.addEventListener('mousedown', handleOutside);
    window.addEventListener('scroll', handleScrollOrResize, true);
    window.addEventListener('resize', handleScrollOrResize);
    return () => {
      document.removeEventListener('mousedown', handleOutside);
      window.removeEventListener('scroll', handleScrollOrResize, true);
      window.removeEventListener('resize', handleScrollOrResize);
    };
  }, [open, updatePosition]);

  const item = (
    label: string,
    icon: React.ReactNode,
    onClick: () => void,
    options?: { danger?: boolean; disabled?: boolean; title?: string },
  ) => {
    const danger = options?.danger ?? false;
    const disabled = options?.disabled ?? false;
    return (
      <button
        type="button"
        disabled={disabled}
        title={options?.title}
        onClick={() => { if (!disabled) { onClick(); setOpen(false); } }}
        className="w-full flex items-center gap-2.5 px-3 py-2 text-xs font-medium text-left transition cursor-pointer disabled:opacity-40 disabled:cursor-not-allowed"
        style={{
          background: 'transparent',
          color: danger ? 'var(--pill-negative-ink)' : 'var(--text)',
          border: 'none',
          borderRadius: '6px',
        }}
        onMouseEnter={e => {
          if (!disabled) e.currentTarget.style.background = danger ? 'rgba(239,68,68,0.14)' : 'var(--active)';
        }}
        onMouseLeave={e => (e.currentTarget.style.background = 'transparent')}
      >
        {icon}
        <span>{label}</span>
      </button>
    );
  };

  return (
    <div style={{ display: 'inline-block' }}>
      <button
        ref={triggerRef}
        type="button"
        onClick={e => {
          e.stopPropagation();
          if (!open) updatePosition();
          setOpen(o => !o);
        }}
        className="flex items-center gap-1 px-2.5 py-1 text-xs font-medium border cursor-pointer transition btn-press"
        style={{
          background: open ? 'var(--primary)' : 'var(--raised)',
          color: open ? '#fff' : 'var(--text)',
          borderColor: open ? 'var(--primary)' : 'var(--border)',
          borderRadius: '6px',
          height: '28px',
        }}
      >
        Manage
        <MoreVertical className="w-3 h-3" />
      </button>

      {open && createPortal(
        <div
          ref={menuRef}
          className={`${pos.isDark ? 'staff-mode ' : ''}manage-dropdown`}
          style={{
            position: 'fixed',
            top: pos.top,
            right: pos.right,
            transform: pos.openUp ? 'translateY(-100%)' : undefined,
            zIndex: 9999,
            background: 'var(--raised)',
            border: '1px solid var(--border)',
            borderRadius: '10px',
            boxShadow: '0 14px 36px rgba(0,0,0,0.48)',
            padding: '5px',
            minWidth: '188px',
            whiteSpace: 'nowrap',
          }}
          onClick={e => e.stopPropagation()}
        >
          {item(
            isPinned ? 'Unpin order' : 'Pin order',
            isPinned ? <PinOff className="w-3.5 h-3.5" /> : <Pin className="w-3.5 h-3.5" />,
            onPin,
          )}
          <div style={{ height: '1px', background: 'var(--border)', margin: '2px 4px', opacity: 0.6 }} />
          {item('View order details', <Eye className="w-3.5 h-3.5" />, onViewDetails)}
          <div style={{ height: '1px', background: 'var(--border)', margin: '2px 4px', opacity: 0.6 }} />
          {item('Delete order', <Trash2 className="w-3.5 h-3.5" />, onDelete, { danger: true })}
        </div>,
        document.body,
      )}
    </div>
  );
};

// ── Delete Confirmation Dialog ────────────────────────────────────────────────
const DeleteConfirmDialog: React.FC<{
  orderId: number;
  onConfirm: () => void;
  onCancel: () => void;
}> = ({ orderId, onConfirm, onCancel }) => createPortal(
  <div
    className="staff-mode"
    style={{
      position: 'fixed', inset: 0, zIndex: 10000,
      background: 'rgba(0,0,0,0.6)',
      display: 'flex', alignItems: 'center', justifyContent: 'center',
      padding: '16px',
    }}
    onClick={onCancel}
  >
    <div
      className="animate-pop-in"
      onClick={e => e.stopPropagation()}
      style={{
        background: 'var(--surface)',
        border: '1px solid var(--border)',
        borderRadius: '16px',
        padding: '24px',
        width: '100%',
        maxWidth: '360px',
        boxShadow: '0 20px 60px rgba(0,0,0,0.45)',
      }}
    >
      <div className="flex items-start justify-between mb-3">
        <div className="flex items-center gap-2">
          <div style={{ background: 'rgba(239,68,68,0.15)', borderRadius: '8px', padding: '6px' }}>
            <Trash2 className="w-4 h-4" style={{ color: '#EF4444' }} />
          </div>
          <h3 className="font-semibold text-sm" style={{ color: 'var(--text)' }}>Delete order #{orderId}?</h3>
        </div>
        <button onClick={onCancel} style={{ background: 'none', border: 'none', cursor: 'pointer', color: 'var(--muted)', padding: 0 }}>
          <X className="w-4 h-4" />
        </button>
      </div>
      <p className="text-xs mb-5 leading-relaxed" style={{ color: 'var(--muted)' }}>
        This will permanently delete order #{orderId} from SQLite and remove it from any active queue. This action cannot be undone.
      </p>
      <div className="flex gap-2 justify-end">
        <button
          onClick={onCancel}
          className="px-4 py-2 text-xs font-medium border cursor-pointer transition"
          style={{ background: 'var(--raised)', color: 'var(--text)', borderColor: 'var(--border)', borderRadius: '8px' }}
        >
          Keep order
        </button>
        <button
          onClick={onConfirm}
          className="px-4 py-2 text-xs font-semibold cursor-pointer transition btn-press"
          style={{ background: '#DC2626', color: '#fff', border: 'none', borderRadius: '8px' }}
        >
          Yes, delete
        </button>
      </div>
    </div>
  </div>,
  document.body,
);

interface Props {
  onGoStudent?: () => void;
}

export const AdminDashboard: React.FC<Props> = ({ onGoStudent }) => {
  const [isStaffAuthenticated, setIsStaffAuthenticated] = useState<boolean>(() =>
    !!(sessionStorage.getItem('canteen_staff_auth') === 'true' && getStaffToken())
  );
  const [staffPin,     setStaffPin]     = useState<string>('');
  const [pinError,     setPinError]     = useState<string | null>(null);
  const [loginLoading, setLoginLoading] = useState<boolean>(false);

  const [selectedOrderId, setSelectedOrderId] = useState<string>('');
  const [currentOrder,    setCurrentOrder]    = useState<Order | null>(null);
  // eslint-disable-next-line @typescript-eslint/no-unused-vars
  const [_counters,       setCounters]        = useState<Counter[]>([]);
  const [analytics,       setAnalytics]       = useState<AnalyticsData | null>(null);
  const [loading,         setLoading]         = useState<boolean>(false);
  const [actionLoading,   setActionLoading]   = useState<string | null>(null);
  const [actionMessage,   setActionMessage]   = useState<{ type: 'success' | 'error'; text: string } | null>(null);
  const [recentOrders,    setRecentOrders]    = useState<RecentOrderMeta[]>([]);
  const [autoRefresh,     setAutoRefresh]     = useState<boolean>(true);
  const actionPanelRef = useRef<HTMLDivElement>(null);

  const [orders,       setOrders]       = useState<Order[]>([]);
  const [totalOrders,  setTotalOrders]  = useState<number>(0);
  const [ordersLoading, setOrdersLoading] = useState<boolean>(true);
  const [ordersError,   setOrdersError]   = useState<string | null>(null);
  const [statusFilter,  setStatusFilter]  = useState<string>('ALL');
  const [currentPage,   setCurrentPage]   = useState<number>(1);
  const pageSize = 10;

  // ── Pinned orders (UI-only, sessionStorage persisted) ──────────────────────
  const [pinnedOrders, setPinnedOrders] = useState<Set<number>>(() => {
    try {
      const saved = sessionStorage.getItem('canteen_pinned_orders');
      return saved ? new Set<number>(JSON.parse(saved)) : new Set<number>();
    } catch {
      return new Set<number>();
    }
  });
  const [deleteConfirm, setDeleteConfirm] = useState<number | null>(null);
  const [tableMessage, setTableMessage] = useState<{ type: 'success' | 'error'; text: string } | null>(null);

  const togglePin = (id: number) => {
    setPinnedOrders(prev => {
      const next = new Set(prev);
      if (next.has(id)) next.delete(id);
      else next.add(id);
      try { sessionStorage.setItem('canteen_pinned_orders', JSON.stringify([...next])); } catch { /* ignore */ }
      return next;
    });
  };

  const handleDeleteOrder = async (id: number) => {
    setDeleteConfirm(null);
    setActionLoading('delete');
    setTableMessage(null);
    try {
      await deleteOrder(id);
      const msg = `Order #${id} permanently deleted.`;
      setActionMessage({ type: 'success', text: msg });
      setTableMessage({ type: 'success', text: msg });
      if (currentOrder?.id === id) setCurrentOrder(null);
      setPinnedOrders(prev => {
        const next = new Set(prev);
        next.delete(id);
        try { sessionStorage.setItem('canteen_pinned_orders', JSON.stringify([...next])); } catch { /* ignore */ }
        return next;
      });
      loadTelemetry();
      loadOrdersTable(true);
    } catch (err: any) {
      const errMsg = err.message || `Could not delete order #${id}.`;
      setActionMessage({ type: 'error', text: errMsg });
      setTableMessage({ type: 'error', text: errMsg });
    } finally { setActionLoading(null); }
  };

  const handleStaffLogin = async (e: React.FormEvent) => {
    e.preventDefault(); setLoginLoading(true); setPinError(null);
    try { await staffLogin(staffPin.trim()); setIsStaffAuthenticated(true); }
    catch (err: any) { setPinError(err.message || 'Invalid staff passcode.'); }
    finally { setLoginLoading(false); }
  };

  const handleStaffLogout = () => {
    setIsStaffAuthenticated(false); clearStaffToken(); setCurrentOrder(null);
  };

  const loadTelemetry = async () => {
    try {
      const [cData, aData] = await Promise.all([getCounters(), getAnalytics()]);
      setCounters(cData); setAnalytics(aData); setRecentOrders(getRecentOrders());
    } catch (err: any) { console.error('Telemetry load failed', err); }
  };

  const loadOrdersTable = useCallback(async (silent = false) => {
    if (!silent) {
      setOrdersLoading(true);
      setOrdersError(null);
    }
    try {
      const offset = (currentPage - 1) * pageSize;
      const resp = await getOrders({ limit: pageSize, offset, status: statusFilter === 'ALL' ? undefined : statusFilter });
      setOrders(resp.orders); setTotalOrders(resp.total);
    } catch (err: any) {
      if (!silent) setOrdersError(err.message || 'Failed to fetch orders');
    } finally {
      if (!silent) setOrdersLoading(false);
    }
  }, [currentPage, statusFilter, pageSize]);

  const currentOrderIdRef = useRef<number | null>(null);
  useEffect(() => { currentOrderIdRef.current = currentOrder?.id ?? null; }, [currentOrder?.id]);

  useEffect(() => { loadTelemetry(); }, []);
  useEffect(() => { loadOrdersTable(false); }, [loadOrdersTable]);

  useEffect(() => {
    if (!autoRefresh) return;
    const interval = setInterval(() => {
      loadTelemetry();
      loadOrdersTable(true);
      const activeId = currentOrderIdRef.current;
      if (activeId !== null) {
        getOrderById(activeId).then(f => setCurrentOrder(f)).catch(() => {});
      }
    }, 4000);
    return () => clearInterval(interval);
  }, [autoRefresh, loadOrdersTable]);

  const fetchOrderDetails = async (id: number) => {
    setLoading(true); setActionMessage(null);
    try {
      const ord = await getOrderById(id);
      setCurrentOrder(ord); updateRecentOrderStatus(ord.id, ord.status); setRecentOrders(getRecentOrders());
    } catch (err: any) {
      setCurrentOrder(null);
      setActionMessage({ type: 'error', text: err.message || `Order #${id} not found.` });
    } finally { setLoading(false); }
  };

  const handleAction = async (action: 'accept' | 'pay' | 'start' | 'ready' | 'collect' | 'cancel') => {
    if (!currentOrder) return;
    setActionLoading(action); setActionMessage(null);
    try {
      let updated: Order;
      if      (action === 'accept')  updated = await acceptOrder(currentOrder.id);
      else if (action === 'pay')     updated = await payOrder(currentOrder.id);
      else if (action === 'start')   updated = await startOrder(currentOrder.id);
      else if (action === 'ready')   updated = await readyOrder(currentOrder.id);
      else if (action === 'collect') updated = await collectOrder(currentOrder.id);
      else                           updated = await cancelOrder(currentOrder.id);

      setCurrentOrder(updated);
      updateRecentOrderStatus(updated.id, updated.status);
      setRecentOrders(getRecentOrders());
      setActionMessage({ type: 'success', text: `Order #${updated.id} → ${updated.status}.` });
      loadTelemetry(); loadOrdersTable(true);
    } catch (err: any) {
      setActionMessage({ type: 'error', text: err.message || `Action ${action} failed.` });
    } finally { setActionLoading(null); }
  };

  const { displayedOrders, fifoPosMap } = useMemo(() => {
    const posMap = new Map<number, number>();
    if (statusFilter === 'QUEUED' || statusFilter === 'PREPARING') {
      orders.forEach((o, i) => posMap.set(o.id, i + 1));
    }
    if (pinnedOrders.size === 0) {
      return { displayedOrders: orders, fifoPosMap: posMap };
    }
    const sorted = [...orders].sort((a, b) => {
      const aPin = pinnedOrders.has(a.id) ? 1 : 0;
      const bPin = pinnedOrders.has(b.id) ? 1 : 0;
      return bPin - aPin;
    });
    return { displayedOrders: sorted, fifoPosMap: posMap };
  }, [orders, pinnedOrders, statusFilter]);

  const totalPages = Math.max(1, Math.ceil(totalOrders / pageSize));
  const filterOptions = [
    { id: 'ALL',       label: 'All orders' },
    { id: 'QUEUED',    label: 'Queued'     },
    { id: 'PREPARING', label: 'Cooking'    },
    { id: 'READY',     label: 'Ready'      },
    { id: 'COLLECTED', label: 'Collected'  },
    { id: 'CANCELLED', label: 'Cancelled'  },
  ];

  // ── Auth gate ────────────────────────────────────────────────────────────────
  if (!isStaffAuthenticated) {
    return (
      <div className="min-h-screen flex items-center justify-center px-4" style={{ background: 'var(--ground)' }}>
        <div className="w-full max-w-sm border animate-fadeIn"
          style={{ background: 'var(--surface)', borderColor: 'var(--border)', borderRadius: '16px' }}>
          <div className="p-6 border-b" style={{ borderColor: 'var(--border)' }}>
            <div className="flex items-center gap-2 mb-1">
              <div
                className="w-8 h-8 rounded-lg flex items-center justify-center"
                style={{ background: 'var(--primary)' }}
              >
                <Lock className="w-4 h-4 text-white" />
              </div>
              <h1 className="text-lg font-bold" style={{ color: 'var(--text)' }}>Staff Access</h1>
            </div>
            <p className="text-xs mt-1" style={{ color: 'var(--muted)' }}>Restricted workspace for kitchen and counter staff.</p>
          </div>
          <div className="p-6 space-y-4">
            {onGoStudent && (
              <button
                onClick={onGoStudent}
                className="flex items-center gap-1 text-xs cursor-pointer mb-2"
                style={{ color: 'var(--muted)', background: 'none', border: 'none', padding: 0 }}
              >
                <ChevronLeft className="w-3.5 h-3.5" />Student view
              </button>
            )}
            {pinError && (
              <div className="flex items-center gap-2 px-3 py-2.5 text-xs" style={{ background: 'rgba(239,68,68,0.15)', color: '#EF4444', borderRadius: '8px' }}>
                <AlertCircle className="w-3.5 h-3.5 shrink-0" /><span>{pinError}</span>
              </div>
            )}
            <form onSubmit={handleStaffLogin} className="space-y-3">
              <div>
                <label className="block text-xs font-medium mb-1.5" style={{ color: 'var(--muted)' }}>Passcode</label>
                <input type="password" required value={staffPin} onChange={e => setStaffPin(e.target.value)} placeholder="Enter staff PIN"
                  className="w-full px-3 text-sm border outline-none transition"
                  style={{ background: 'var(--raised)', borderColor: 'var(--border)', color: 'var(--text)', borderRadius: '8px', height: '42px' }} />
                <p className="text-xs mt-1.5" style={{ color: 'var(--faint)' }}>Contact the canteen manager if you do not have a PIN.</p>
              </div>
              <button type="submit" disabled={loginLoading}
                className="w-full py-2.5 font-medium text-sm text-white flex items-center justify-center gap-2 cursor-pointer disabled:opacity-60 transition"
                style={{ background: 'var(--primary)', borderRadius: '8px' }}>
                {loginLoading ? <Loader2 className="w-4 h-4 animate-spin" /> : <Unlock className="w-4 h-4" />}
                {loginLoading ? 'Verifying…' : 'Sign in'}
              </button>
            </form>
          </div>
        </div>
      </div>
    );
  }

  // ── Ops console ───────────────────────────────────────────────────────────────
  return (
    <div className="animate-fadeIn">
      {/* Sticky header */}
      <header
        className="sticky top-0 z-50 border-b"
        style={{ background: 'var(--surface)', borderColor: 'var(--border)' }}
      >
        <div className="max-w-7xl mx-auto px-4 sm:px-6 h-14 flex items-center justify-between gap-4">
          <div className="flex items-center gap-4">
            {onGoStudent && (
              <>
                <button
                  onClick={onGoStudent}
                  className="flex items-center gap-1 text-xs font-medium transition cursor-pointer"
                  style={{ color: 'var(--muted)', background: 'none', border: 'none', padding: 0 }}
                >
                  <ChevronLeft className="w-4 h-4" />
                  <span className="hidden sm:inline">Student app</span>
                </button>
                <div className="w-px h-5" style={{ background: 'var(--border)' }} />
              </>
            )}
            <div className="flex items-center gap-2">
              <span
                className="w-7 h-7 rounded flex items-center justify-center text-xs font-bold text-white select-none"
                style={{ background: 'var(--primary)' }}
              >
                <ChefHat className="w-4 h-4" />
              </span>
              <div>
                <div className="text-sm font-semibold leading-none" style={{ color: 'var(--text)' }}>Kitchen Ops</div>
                <div className="text-[10px] mt-0.5" style={{ color: 'var(--muted)' }}>Canteen staff dashboard</div>
              </div>
            </div>
          </div>
          <div className="flex items-center gap-2">
            <button type="button" onClick={() => setAutoRefresh(!autoRefresh)}
              className="flex items-center gap-1 text-xs font-medium px-2 py-1 border cursor-pointer transition"
              style={{ background: autoRefresh ? 'var(--raised)' : 'var(--surface)', color: autoRefresh ? 'var(--positive)' : 'var(--muted)', borderColor: 'var(--border)', borderRadius: '6px' }}>
              <span className="w-1.5 h-1.5 rounded-full" style={{ background: autoRefresh ? 'var(--positive)' : 'var(--faint)' }} />
              {autoRefresh ? 'Live' : 'Paused'}
            </button>
            <button onClick={() => { loadTelemetry(); loadOrdersTable(false); }}
              className="flex items-center gap-1.5 px-3 py-1.5 text-xs font-medium border cursor-pointer transition"
              style={{ background: 'var(--raised)', borderColor: 'var(--border)', color: 'var(--muted)', borderRadius: '6px' }}>
              <RefreshCw className="w-3.5 h-3.5" />Refresh
            </button>
            <button onClick={handleStaffLogout}
              className="flex items-center gap-1.5 px-3 py-1.5 text-xs font-medium cursor-pointer transition border"
              style={{ background: 'transparent', color: 'var(--primary)', borderColor: 'var(--primary)', borderRadius: '6px' }}>
              <Lock className="w-3.5 h-3.5" />Sign out
            </button>
          </div>
        </div>
      </header>

      <div className="max-w-7xl mx-auto px-4 sm:px-6 py-6 space-y-6 pb-16">
        {/* Page heading */}
        <div>
          <h1 className="text-2xl font-bold" style={{ color: 'var(--text)' }}>Kitchen operations</h1>
          <p className="text-sm mt-1" style={{ color: 'var(--muted)' }}>Advance order states, verify payment, and manage the C++ queue.</p>
        </div>

        {/* KPI strip */}
        {analytics && (
          <div className="border flex flex-col sm:flex-row" style={{ background: 'var(--surface)', borderColor: 'var(--border)', borderRadius: '12px' }}>
            {[
              { label: 'Total orders',    value: analytics.total_orders_history_count,         accent: false, small: false },
              { label: 'In queue',        value: analytics.currently_pending_orders,            accent: true,  small: false },
              { label: 'Active workload', value: `${analytics.total_active_workload_minutes}m`, accent: false, small: false },
              { label: 'ML engine',       value: analytics.ml_model_active ? 'Random Forest (20 trees)' : 'Baseline sum', accent: false, small: true },
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

        {/* Order dispatch */}
        <div ref={actionPanelRef} className="border" style={{ background: 'var(--surface)', borderColor: 'var(--border)', borderRadius: '12px' }}>
          <div className="px-5 py-4 border-b" style={{ borderColor: 'var(--border)' }}>
            <h2 className="text-base font-semibold" style={{ color: 'var(--text)' }}>Order dispatch</h2>
            <p className="text-xs mt-0.5" style={{ color: 'var(--muted)' }}>Search by order ID or click Manage on any row below.</p>
          </div>

          <div className="p-5 space-y-4">
            {/* Search */}
            <form onSubmit={e => { e.preventDefault(); if (selectedOrderId) fetchOrderDetails(Number(selectedOrderId)); }} className="flex gap-2">
              <div className="relative flex-1">
                <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-3.5 h-3.5" style={{ color: 'var(--faint)' }} />
                <input type="number" placeholder="Order ID (e.g. 101)" value={selectedOrderId} onChange={e => setSelectedOrderId(e.target.value)}
                  className="w-full pl-9 pr-3 text-sm border outline-none transition"
                  style={{ background: 'var(--raised)', borderColor: 'var(--border)', color: 'var(--text)', borderRadius: '6px', height: '38px' }} />
              </div>
              <button type="submit" disabled={loading}
                className="px-4 text-sm font-medium text-white cursor-pointer disabled:opacity-50 flex items-center gap-2 transition"
                style={{ background: 'var(--primary)', borderRadius: '6px', height: '38px' }}>
                {loading ? <Loader2 className="w-3.5 h-3.5 animate-spin" /> : <Search className="w-3.5 h-3.5" />}
                Inspect
              </button>
            </form>

            {/* Recent IDs */}
            {recentOrders.length > 0 && (
              <div className="flex items-center gap-2 overflow-x-auto text-xs scrollbar-none">
                <span className="font-medium shrink-0" style={{ color: 'var(--muted)' }}>Recent:</span>
                {recentOrders.slice(0, 8).map(ro => (
                  <button key={ro.id} onClick={() => { setSelectedOrderId(String(ro.id)); fetchOrderDetails(ro.id); }}
                    className="px-2.5 py-1 border font-mono font-medium transition shrink-0 cursor-pointer"
                    style={{ background: currentOrder?.id === ro.id ? 'var(--primary)' : 'var(--raised)', color: currentOrder?.id === ro.id ? '#fff' : 'var(--text)', borderColor: currentOrder?.id === ro.id ? 'var(--primary)' : 'var(--border)', borderRadius: '6px' }}>
                    #{ro.id}
                  </button>
                ))}
              </div>
            )}

            {/* Action result */}
            {actionMessage && (
              <div className="flex items-center gap-2 px-3 py-2.5 text-xs"
                style={{ background: actionMessage.type === 'success' ? 'rgba(34,197,94,0.15)' : 'rgba(239,68,68,0.15)', color: actionMessage.type === 'success' ? '#22C55E' : '#EF4444', borderRadius: '6px' }}>
                {actionMessage.type === 'success' ? <CheckCircle2 className="w-3.5 h-3.5 shrink-0" /> : <AlertCircle className="w-3.5 h-3.5 shrink-0" />}
                <span className="font-medium">{actionMessage.text}</span>
              </div>
            )}

            {/* Ticket */}
            {currentOrder ? (
              <div className="border" style={{ background: 'var(--raised)', borderColor: 'var(--border)', borderRadius: '8px' }}>
                <div className="px-5 py-4 border-b flex flex-col sm:flex-row sm:items-center justify-between gap-3" style={{ borderColor: 'var(--border)' }}>
                  <div>
                    <div className="flex flex-wrap items-center gap-2">
                      <span className="font-semibold text-base num" style={{ color: 'var(--text)' }}>Order #{currentOrder.id}</span>
                      <StatusBadge status={currentOrder.status} size="sm" />
                      <PaymentBadge status={currentOrder.payment_status} size="sm" />
                    </div>
                    <div className="text-xs mt-1" style={{ color: 'var(--muted)' }}>
                      {currentOrder.student_name} ({currentOrder.student_id}) · Counter #{currentOrder.assigned_counter_id} · ~{currentOrder.total_prep_time_minutes}m
                    </div>
                  </div>
                  <div className="text-right">
                    <span className="text-xs" style={{ color: 'var(--muted)' }}>Total</span>
                    <div className="font-semibold text-lg num" style={{ color: 'var(--text)' }}>₹{currentOrder.total_amount.toFixed(2)}</div>
                  </div>
                </div>

                <div className="px-5 py-3 flex flex-wrap gap-1.5">
                  {currentOrder.items.map((line, idx) => (
                    <span key={idx} className="px-2 py-1 text-xs border"
                      style={{ background: 'var(--surface)', color: 'var(--text)', borderColor: 'var(--border)', borderRadius: '6px' }}>
                      {line.quantity}× {line.item_name}
                    </span>
                  ))}
                </div>

                <div className="px-5 pb-4">
                  <div className="flex items-center justify-between mb-2">
                    <span className="text-xs font-medium" style={{ color: 'var(--muted)' }}>Actions</span>
                    <span className="text-xs" style={{ color: 'var(--faint)' }}>Status: <strong style={{ color: 'var(--text)' }}>{currentOrder.status}</strong></span>
                  </div>
                  <div className="flex flex-wrap gap-2">
                    <ActionBtn label="Accept" icon={<Check className="w-3.5 h-3.5" />}
                      disabled={currentOrder.status !== 'QUEUED' || actionLoading !== null}
                      loading={actionLoading === 'accept'} onClick={() => handleAction('accept')} />
                    <ActionBtn label={currentOrder.payment_status === 'PAID' ? 'Paid' : 'Mark paid'} icon={<CreditCard className="w-3.5 h-3.5" />}
                      disabled={currentOrder.payment_status === 'PAID' || currentOrder.status === 'CANCELLED' || actionLoading !== null}
                      loading={actionLoading === 'pay'} onClick={() => handleAction('pay')} />
                    <ActionBtn label="Start cooking" icon={<Play className="w-3.5 h-3.5" />}
                      disabled={currentOrder.status !== 'QUEUED' || actionLoading !== null}
                      loading={actionLoading === 'start'} onClick={() => handleAction('start')} primary />
                    <ActionBtn label="Mark ready" icon={<CheckCircle2 className="w-3.5 h-3.5" />}
                      disabled={currentOrder.status !== 'PREPARING' || actionLoading !== null}
                      loading={actionLoading === 'ready'} onClick={() => handleAction('ready')} primary />
                    <ActionBtn label="Collect" icon={<PackageCheck className="w-3.5 h-3.5" />}
                      disabled={currentOrder.status !== 'READY' || actionLoading !== null}
                      loading={actionLoading === 'collect'} onClick={() => handleAction('collect')} primary />
                    <ActionBtn label="Cancel" icon={<XCircle className="w-3.5 h-3.5" />}
                      disabled={currentOrder.status === 'COLLECTED' || currentOrder.status === 'CANCELLED' || actionLoading !== null}
                      loading={actionLoading === 'cancel'} onClick={() => handleAction('cancel')} danger />
                  </div>
                </div>
              </div>
            ) : (
              !loading && (
                <div className="py-8 text-center text-sm border"
                  style={{ color: 'var(--muted)', borderColor: 'var(--border)', borderStyle: 'dashed', borderRadius: '8px' }}>
                  No order selected. Click Manage on any row below, or search by ID.
                </div>
              )
            )}
          </div>
        </div>

        {/* Orders table */}
        <div className="border" style={{ background: 'var(--surface)', borderColor: 'var(--border)', borderRadius: '12px' }}>
          <div className="px-5 py-4 border-b flex flex-col sm:flex-row sm:items-center justify-between gap-3" style={{ borderColor: 'var(--border)' }}>
            <div>
              <h2 className="text-base font-semibold" style={{ color: 'var(--text)' }}>Persisted orders</h2>
              <p className="text-xs mt-0.5" style={{ color: 'var(--muted)' }}>
                Live SQLite query via <code className="font-mono" style={{ color: 'var(--primary)' }}>GET /api/orders</code>
              </p>
            </div>
            <div className="flex items-center gap-2">
              <span className="text-xs" style={{ color: 'var(--muted)' }}>Total: <strong style={{ color: 'var(--text)' }}>{totalOrders}</strong></span>
              <button onClick={() => loadOrdersTable(false)} className="p-1.5 cursor-pointer transition" style={{ color: 'var(--muted)', background: 'none', border: 'none', borderRadius: '4px' }}>
                <RefreshCw className={`w-3.5 h-3.5 ${ordersLoading ? 'animate-spin' : ''}`} />
              </button>
            </div>
          </div>

          {/* Filter tabs */}
          <div className="px-5 py-3 border-b flex items-center gap-2 overflow-x-auto scrollbar-none" style={{ borderColor: 'var(--border)' }}>
            <Filter className="w-3.5 h-3.5 shrink-0" style={{ color: 'var(--faint)' }} />
            {filterOptions.map(opt => {
              const active = statusFilter === opt.id;
              return (
                <button key={opt.id} onClick={() => { setStatusFilter(opt.id); setCurrentPage(1); }}
                  className="px-3 py-1 text-xs font-medium whitespace-nowrap cursor-pointer transition"
                  style={{ background: active ? 'var(--primary)' : 'var(--raised)', color: active ? '#fff' : 'var(--muted)', border: '1px solid', borderColor: active ? 'var(--primary)' : 'var(--border)', borderRadius: '6px' }}>
                  {opt.label}
                </button>
              );
            })}
          </div>

          {tableMessage && (
            <div className="px-5 pt-3">
              <div
                className="flex items-center justify-between gap-2 px-3 py-2 text-xs"
                style={{
                  background: tableMessage.type === 'success' ? 'rgba(34,197,94,0.14)' : 'rgba(239,68,68,0.15)',
                  color: tableMessage.type === 'success' ? '#4ADE80' : '#FC8181',
                  border: '1px solid',
                  borderColor: tableMessage.type === 'success' ? 'rgba(34,197,94,0.3)' : 'rgba(239,68,68,0.35)',
                  borderRadius: '6px',
                }}
              >
                <div className="flex items-center gap-2">
                  {tableMessage.type === 'success'
                    ? <CheckCircle2 className="w-3.5 h-3.5 shrink-0" />
                    : <AlertCircle className="w-3.5 h-3.5 shrink-0" />}
                  <span>{tableMessage.text}</span>
                </div>
                <button
                  type="button"
                  onClick={() => setTableMessage(null)}
                  className="opacity-70 hover:opacity-100 cursor-pointer"
                  style={{ background: 'none', border: 'none', color: 'inherit', padding: 0 }}
                >
                  <X className="w-3.5 h-3.5" />
                </button>
              </div>
            </div>
          )}

          {ordersError && (
            <div className="p-4">
              <div className="flex items-center justify-between p-3 text-xs" style={{ background: 'rgba(239,68,68,0.15)', color: '#EF4444', borderRadius: '6px' }}>
                <div className="flex items-center gap-2"><AlertCircle className="w-3.5 h-3.5 shrink-0" />{ordersError}</div>
                <button onClick={() => loadOrdersTable(false)} className="font-medium underline cursor-pointer">Retry</button>
              </div>
            </div>
          )}

          {/* Table */}
          <div className="overflow-x-auto">
            <table className="w-full text-left text-xs" style={{ color: 'var(--text)' }}>
              <thead>
                <tr style={{ borderBottom: '1px solid var(--border)', background: 'var(--raised)' }}>
                  {['Order ID','Student','Counter','Items','Amount','Payment','Status','Prep','Placed','Action'].map((h, i) => (
                    <th key={h} className={`py-2.5 px-4 font-medium ${i === 4 ? 'text-right' : i >= 5 && i <= 8 ? 'text-center' : ''}`} style={{ color: 'var(--muted)' }}>{h}</th>
                  ))}
                </tr>
              </thead>
              <tbody>
                {ordersLoading ? (
                  <tr><td colSpan={10} className="py-8 text-center" style={{ color: 'var(--muted)' }}>
                    <div className="flex items-center justify-center gap-2"><Loader2 className="w-4 h-4 animate-spin" />Loading orders…</div>
                  </td></tr>
                ) : displayedOrders.length === 0 ? (
                  <tr><td colSpan={10} className="py-8 text-center" style={{ color: 'var(--muted)' }}>
                    No orders{statusFilter !== 'ALL' ? ` with status ${statusFilter}` : ''}.
                  </td></tr>
                ) : displayedOrders.map((ord) => {
                  const isSelected = currentOrder?.id === ord.id;
                  const isPinned   = pinnedOrders.has(ord.id);
                  const fifoPos = fifoPosMap.get(ord.id) ?? null;
                  const isNext  = fifoPos === 1;
                  return (
                    <tr key={ord.id}
                      className={isPinned ? 'pinned-row' : undefined}
                      style={{
                        borderBottom: '1px solid var(--border)',
                        background: isSelected ? 'var(--active)' : isPinned ? undefined : 'transparent',
                        borderLeft: isSelected || isNext ? '2px solid var(--primary)' : isPinned ? '3px solid #F59E0B' : '2px solid transparent',
                        cursor: 'pointer',
                        transition: 'background 0.2s ease',
                      }}
                      onClick={() => { setSelectedOrderId(String(ord.id)); fetchOrderDetails(ord.id); }}>
                      <td className="py-3 px-4 font-mono font-medium">
                        <div className="flex items-center gap-1.5">
                          #{ord.id}
                          {isPinned && (
                            <span className="pinned-badge">📌 Pinned</span>
                          )}
                          {fifoPos !== null && (
                            <span className="text-[9px] font-bold px-1.5 py-0.5"
                              style={isNext ? { background: 'var(--primary)', color: '#fff', borderRadius: '3px' } : { color: 'var(--primary)' }}>
                              {isNext ? '▶ NEXT' : `#${fifoPos}`}
                            </span>
                          )}
                        </div>
                      </td>
                      <td className="py-3 px-4">
                        <div className="font-medium">{ord.student_name}</div>
                        <div style={{ color: 'var(--faint)' }}>{ord.student_id}</div>
                      </td>
                      <td className="py-3 px-4" style={{ color: 'var(--muted)' }}>Counter #{ord.assigned_counter_id}</td>
                      <td className="py-3 px-4 max-w-[160px] truncate" style={{ color: 'var(--muted)' }}
                        title={ord.items.map(i => `${i.quantity}x ${i.item_name}`).join(', ')}>
                        {ord.items.map(i => `${i.quantity}× ${i.item_name}`).join(', ')}
                      </td>
                      <td className="py-3 px-4 text-right font-medium num">₹{ord.total_amount.toFixed(2)}</td>
                      <td className="py-3 px-4 text-center"><PaymentBadge status={ord.payment_status} size="sm" /></td>
                      <td className="py-3 px-4 text-center"><StatusBadge status={ord.status} size="sm" /></td>
                      <td className="py-3 px-4 text-center font-mono" style={{ color: 'var(--muted)' }}>~{ord.total_prep_time_minutes}m</td>
                      <td className="py-3 px-4 text-right" style={{ color: 'var(--muted)' }}>
                        {new Date(ord.order_timestamp * 1000).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}
                      </td>
                      <td className="py-3 px-4 text-center" style={{ position: 'relative' }}>
                        <ManageDropdown
                          orderId={ord.id}
                          isPinned={isPinned}
                          onPin={() => togglePin(ord.id)}
                          onDelete={() => setDeleteConfirm(ord.id)}
                          onViewDetails={() => {
                            setSelectedOrderId(String(ord.id));
                            fetchOrderDetails(ord.id);
                            setTimeout(() => actionPanelRef.current?.scrollIntoView({ behavior: 'smooth', block: 'start' }), 80);
                          }}
                        />
                      </td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </div>

          {/* Pagination */}
          {totalOrders > 0 && (
            <div className="p-3.5 border-t flex items-center justify-between text-xs"
              style={{ borderColor: 'var(--border)', background: 'var(--raised)', color: 'var(--muted)', borderRadius: '0 0 12px 12px' }}>
              <div>
                Showing <strong style={{ color: 'var(--text)' }}>{(currentPage - 1) * pageSize + 1}</strong> to{' '}
                <strong style={{ color: 'var(--text)' }}>{Math.min(currentPage * pageSize, totalOrders)}</strong> of{' '}
                <strong style={{ color: 'var(--text)' }}>{totalOrders}</strong>
              </div>
              <div className="flex items-center gap-1">
                <button onClick={() => setCurrentPage(p => Math.max(1, p - 1))} disabled={currentPage <= 1 || ordersLoading}
                  className="p-1.5 border cursor-pointer disabled:opacity-40 transition"
                  style={{ background: 'var(--surface)', borderColor: 'var(--border)', color: 'var(--text)', borderRadius: '6px' }}>
                  <ChevronLeft className="w-4 h-4" />
                </button>
                <span className="px-2 font-medium" style={{ color: 'var(--text)' }}>{currentPage} / {totalPages}</span>
                <button onClick={() => setCurrentPage(p => Math.min(totalPages, p + 1))} disabled={currentPage >= totalPages || ordersLoading}
                  className="p-1.5 border cursor-pointer disabled:opacity-40 transition"
                  style={{ background: 'var(--surface)', borderColor: 'var(--border)', color: 'var(--text)', borderRadius: '6px' }}>
                  <ChevronRight className="w-4 h-4" />
                </button>
              </div>
            </div>
          )}
        </div>
      </div>

      {/* Delete confirmation dialog */}
      {deleteConfirm !== null && (
        <DeleteConfirmDialog
          orderId={deleteConfirm}
          onConfirm={() => handleDeleteOrder(deleteConfirm)}
          onCancel={() => setDeleteConfirm(null)}
        />
      )}
    </div>
  );
};
