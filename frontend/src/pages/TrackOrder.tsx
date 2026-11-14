import React, { useState, useEffect } from 'react';
import {
  Search,
  Clock,
  MapPin,
  User,
  Hash,
  AlertCircle,
  XCircle,
  CheckCircle2,
  ChefHat,
  PackageCheck,
  Loader2,
  Calendar,
  Layers,
  Radio,
} from 'lucide-react';
import type { Order, Counter, RecentOrderMeta } from '../types';
import { getOrderById, cancelOrder } from '../services/api';
import { StatusBadge, PaymentBadge } from '../components/StatusBadge';
import { getRecentOrders, updateRecentOrderStatus } from '../utils/storage';

interface Props {
  initialOrderId?: number | null;
  counters: Counter[];
}

export const TrackOrder: React.FC<Props> = ({ initialOrderId, counters }) => {
  const [searchId, setSearchId] = useState<string>(
    initialOrderId ? String(initialOrderId) : ''
  );
  const [order, setOrder] = useState<Order | null>(null);
  const [loading, setLoading] = useState<boolean>(false);
  const [cancelling, setCancelling] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const [recentOrders, setRecentOrders] = useState<RecentOrderMeta[]>([]);
  const [lastRefreshed, setLastRefreshed] = useState<Date>(new Date());

  useEffect(() => {
    setRecentOrders(getRecentOrders());
    if (initialOrderId) {
      handleSearch(initialOrderId);
    }
  }, [initialOrderId]);

  // Real-time polling: Every 4 seconds, if an order is active, refresh its status from backend
  useEffect(() => {
    if (!order || order.status === 'COLLECTED' || order.status === 'CANCELLED') {
      return;
    }

    const interval = setInterval(async () => {
      try {
        const fresh = await getOrderById(order.id);
        setOrder(fresh);
        updateRecentOrderStatus(fresh.id, fresh.status);
        setLastRefreshed(new Date());
      } catch (err) {
        // Silently skip poll failure
      }
    }, 4000);

    return () => clearInterval(interval);
  }, [order?.id, order?.status]);

  const handleSearch = async (idToSearch?: number) => {
    const targetId = idToSearch || Number(searchId.trim());
    if (!targetId || isNaN(targetId)) {
      setError('Please enter a valid numeric Order ID (e.g. 101).');
      return;
    }

    setLoading(true);
    setError(null);
    try {
      const data = await getOrderById(targetId);
      setOrder(data);
      updateRecentOrderStatus(data.id, data.status);
      setRecentOrders(getRecentOrders());
      setLastRefreshed(new Date());
    } catch (err: any) {
      console.error('Order fetch failed', err);
      setOrder(null);
      setError(err.message || `Order #${targetId} not found.`);
    } finally {
      setLoading(false);
    }
  };

  const handleCancelOrder = async () => {
    if (!order) return;
    if (!window.confirm(`Are you sure you want to cancel Order #${order.id}?`)) {
      return;
    }

    setCancelling(true);
    try {
      const updated = await cancelOrder(order.id);
      setOrder(updated);
      updateRecentOrderStatus(updated.id, updated.status);
      setRecentOrders(getRecentOrders());
    } catch (err: any) {
      alert(`Failed to cancel order: ${err.message}`);
    } finally {
      setCancelling(false);
    }
  };

  const assignedCounter = order
    ? counters.find((c) => c.id === order.assigned_counter_id)
    : null;

  // Lifecycle Stepper configuration
  const steps = [
    { key: 'QUEUED', label: 'Order Queued', icon: Clock, desc: 'Allocated to counter FIFO queue' },
    { key: 'PREPARING', label: 'Cooking', icon: ChefHat, desc: 'In kitchen preparation' },
    { key: 'READY', label: 'Ready for Pickup', icon: CheckCircle2, desc: 'Waiting at counter' },
    { key: 'COLLECTED', label: 'Collected', icon: PackageCheck, desc: 'Handed over to student' },
  ];

  const getStepStatus = (stepKey: string) => {
    if (!order) return 'upcoming';
    if (order.status === 'CANCELLED') return 'cancelled';

    const orderStatusRank: Record<string, number> = {
      QUEUED: 0,
      PREPARING: 1,
      READY: 2,
      COLLECTED: 3,
    };

    const currentRank = orderStatusRank[order.status] ?? 0;
    const thisRank = orderStatusRank[stepKey] ?? 0;

    if (thisRank < currentRank) return 'completed';
    if (thisRank === currentRank) return 'active';
    return 'upcoming';
  };

  return (
    <div className="max-w-4xl mx-auto space-y-7 animate-fadeIn pb-16">
      {/* Title */}
      <div>
        <h1 className="font-serif text-3xl font-extrabold text-[#33251D] tracking-tight">
          Track Your Order
        </h1>
        <p className="text-xs sm:text-sm text-[#81746A] mt-1">
          Monitor your meal in real-time as it advances through counter queue allocation and kitchen preparation.
        </p>
      </div>

      {/* Search Bar & Recent Order Shortcuts */}
      <div className="p-5 rounded-3xl bg-[#FFFDF8] border border-[#E9DED1] shadow-xs space-y-3.5">
        <form
          onSubmit={(e) => {
            e.preventDefault();
            handleSearch();
          }}
          className="flex gap-2"
        >
          <div className="relative flex-1">
            <Search className="w-4 h-4 absolute left-3.5 top-1/2 -translate-y-1/2 text-[#81746A]" />
            <input
              type="number"
              placeholder="Enter Order ID (e.g. 101)..."
              value={searchId}
              onChange={(e) => setSearchId(e.target.value)}
              className="w-full pl-10 pr-4 py-2.5 rounded-xl bg-[#F8F1E8]/50 border border-[#E9DED1] text-sm text-[#33251D] placeholder-[#81746A] focus:outline-none focus:border-[#C95725] focus:bg-[#FFFDF8] transition"
            />
          </div>
          <button
            type="submit"
            disabled={loading}
            className="px-6 py-2.5 rounded-xl bg-[#C95725] hover:bg-[#B3471A] text-white font-bold text-sm flex items-center gap-2 transition disabled:opacity-50 shadow-xs cursor-pointer"
          >
            {loading ? <Loader2 className="w-4 h-4 animate-spin" /> : <Search className="w-4 h-4" />}
            <span>Track</span>
          </button>
        </form>

        {/* Recent Orders Quick Buttons */}
        {recentOrders.length > 0 && (
          <div className="flex items-center gap-2 overflow-x-auto pt-1 text-xs text-[#81746A] scrollbar-none">
            <span className="font-bold text-[#33251D] shrink-0">Recent Orders:</span>
            {recentOrders.map((ro) => (
              <button
                key={ro.id}
                type="button"
                onClick={() => {
                  setSearchId(String(ro.id));
                  handleSearch(ro.id);
                }}
                className={`px-3 py-1 rounded-xl border text-xs font-mono font-bold transition shrink-0 flex items-center gap-1.5 cursor-pointer ${
                  order?.id === ro.id
                    ? 'bg-[#C95725] text-white border-[#C95725] shadow-xs'
                    : 'bg-[#F8F1E8] hover:bg-[#E9DED1] text-[#33251D] border-[#E9DED1]'
                }`}
              >
                <span>#{ro.id}</span>
                <span className={`text-[10px] ${order?.id === ro.id ? 'text-white/80' : 'text-[#81746A]'}`}>
                  ({ro.status})
                </span>
              </button>
            ))}
          </div>
        )}
      </div>

      {/* Error state */}
      {error && !loading && (
        <div className="p-4 rounded-2xl bg-[#FDF0F0] border border-[#F4CECE] text-xs text-[#9E3939] flex items-center gap-2.5">
          <AlertCircle className="w-4 h-4 text-[#9E3939] shrink-0" />
          <span>{error}</span>
        </div>
      )}

      {/* Order Details View */}
      {order && (
        <div className="space-y-6">
          <div className="rounded-3xl bg-[#FFFDF8] border border-[#E9DED1] p-6 sm:p-8 space-y-6 shadow-xs">
            {/* Top Bar: Order ID, Timestamp & Badges */}
            <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-5 border-b border-[#E9DED1]">
              <div className="space-y-1">
                <div className="flex flex-wrap items-center gap-3">
                  <h2 className="font-serif text-2xl sm:text-3xl font-extrabold text-[#33251D] tracking-tight">
                    Order #{order.id}
                  </h2>
                  <StatusBadge status={order.status} size="md" />
                  <PaymentBadge status={order.payment_status} size="md" />
                </div>
                <div className="flex items-center gap-3 text-xs text-[#81746A] font-medium pt-0.5">
                  <span className="flex items-center gap-1 text-[#33251D]">
                    <User className="w-3.5 h-3.5 text-[#81746A]" />
                    {order.student_name} ({order.student_id})
                  </span>
                  <span>•</span>
                  <span className="flex items-center gap-1">
                    <Calendar className="w-3.5 h-3.5 text-[#81746A]" />
                    {new Date(order.order_timestamp * 1000).toLocaleTimeString([], {
                      hour: '2-digit',
                      minute: '2-digit',
                    })}
                  </span>
                  <span>•</span>
                  <span className="flex items-center gap-1 text-[11px] text-[#2D663E]">
                    <Radio className="w-3 h-3 text-[#2D663E] animate-pulse" />
                    Live (Sync: {lastRefreshed.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' })})
                  </span>
                </div>
              </div>

              {/* Cancel Button if eligible */}
              {(order.status === 'QUEUED' || order.status === 'PREPARING') && (
                <button
                  onClick={handleCancelOrder}
                  disabled={cancelling}
                  className="inline-flex items-center gap-1.5 px-3.5 py-2 rounded-xl bg-[#FDF0F0] hover:bg-[#FBE5E5] text-[#9E3939] border border-[#F4CECE] text-xs font-bold transition disabled:opacity-50 self-start sm:self-auto cursor-pointer"
                >
                  {cancelling ? (
                    <Loader2 className="w-3.5 h-3.5 animate-spin" />
                  ) : (
                    <XCircle className="w-3.5 h-3.5 text-[#9E3939]" />
                  )}
                  <span>Cancel Order</span>
                </button>
              )}
            </div>

            {/* Stepper Progress Bar (if not cancelled) */}
            {order.status !== 'CANCELLED' ? (
              <div className="py-2">
                <div className="text-xs font-bold uppercase tracking-wider text-[#81746A] mb-3">
                  Preparation Progress
                </div>
                <div className="grid grid-cols-2 md:grid-cols-4 gap-3 relative">
                  {steps.map((st, i) => {
                    const statusState = getStepStatus(st.key);
                    const Icon = st.icon;

                    const styles = {
                      completed: {
                        circle: 'bg-[#2D663E] text-white border-[#2D663E]',
                        text: 'text-[#2D663E] font-bold',
                        card: 'bg-[#EFF8F1] border-[#CFEAD5]',
                      },
                      active: {
                        circle: 'bg-[#C95725] text-white border-[#C95725] ring-4 ring-[#FDF0E9]',
                        text: 'text-[#C95725] font-extrabold',
                        card: 'bg-[#FDF0E9] border-[#F4D1BD] shadow-xs',
                      },
                      upcoming: {
                        circle: 'bg-[#F8F1E8] text-[#81746A] border-[#E9DED1]',
                        text: 'text-[#81746A] font-medium',
                        card: 'bg-[#FFFDF8] border-[#E9DED1]',
                      },
                      cancelled: {
                        circle: 'bg-[#9E3939] text-white border-[#9E3939]',
                        text: 'text-[#9E3939] font-bold',
                        card: 'bg-[#FDF0F0] border-[#F4CECE]',
                      },
                    }[statusState];

                    return (
                      <div
                        key={st.key}
                        className={`p-3.5 rounded-2xl border ${styles.card} transition flex flex-col justify-between`}
                      >
                        <div className="flex items-center justify-between mb-2">
                          <span className="text-[10px] font-bold uppercase tracking-wider text-[#81746A]">
                            Step {i + 1}
                          </span>
                          <div
                            className={`w-7 h-7 rounded-full flex items-center justify-center border shadow-xs ${styles.circle}`}
                          >
                            <Icon className="w-3.5 h-3.5" />
                          </div>
                        </div>
                        <div>
                          <div className={`text-xs ${styles.text}`}>{st.label}</div>
                          <div className="text-[11px] text-[#81746A] mt-0.5">{st.desc}</div>
                        </div>
                      </div>
                    );
                  })}
                </div>
              </div>
            ) : (
              <div className="p-4 rounded-2xl bg-[#FDF0F0] border border-[#F4CECE] text-[#9E3939] text-xs flex items-center gap-3">
                <XCircle className="w-5 h-5 text-[#9E3939] shrink-0" />
                <div>
                  <div className="font-bold">This order has been cancelled</div>
                  <div className="text-[11px] text-[#9E3939] mt-0.5">
                    Queue spot released and kitchen workload deducted from canteen counters.
                  </div>
                </div>
              </div>
            )}

            {/* Quick Operational Telemetry Summary Grid */}
            <div className="grid grid-cols-1 sm:grid-cols-3 gap-3.5 pt-1">
              {/* Pickup Counter */}
              <div className="p-4 rounded-2xl bg-[#F8F1E8]/70 border border-[#E9DED1]">
                <div className="text-xs font-bold text-[#81746A] flex items-center gap-1.5 mb-1">
                  <MapPin className="w-3.5 h-3.5 text-[#C95725]" />
                  <span>Assigned Counter</span>
                </div>
                <div className="text-base font-bold text-[#33251D]">
                  {assignedCounter ? assignedCounter.name : `Counter #${order.assigned_counter_id}`}
                </div>
                <div className="text-[11px] text-[#81746A] mt-0.5">
                  Ground Floor Canteen
                </div>
              </div>

              {/* Live Queue Position & Wait Time */}
              <div className="p-4 rounded-2xl bg-[#F8F1E8]/70 border border-[#E9DED1]">
                <div className="text-xs font-bold text-[#81746A] flex items-center gap-1.5 mb-1">
                  <Layers className="w-3.5 h-3.5 text-[#C95725]" />
                  <span>Queue Position & Wait</span>
                </div>
                <div className="text-base font-bold text-[#33251D] flex items-center gap-1.5">
                  {order.queue_position && order.queue_position > 0 ? (
                    <span className="text-[#C95725]">Position #{order.queue_position}</span>
                  ) : order.status === 'READY' ? (
                    <span className="text-[#2D663E]">Ready for Pickup!</span>
                  ) : order.status === 'COLLECTED' ? (
                    <span className="text-[#55473D]">Completed</span>
                  ) : (
                    <span>Queue Active</span>
                  )}
                </div>
                <div className="text-[11px] text-[#81746A] mt-0.5 flex items-center gap-1">
                  <Clock className="w-3 h-3 text-[#C95725]" />
                  <span>~{order.total_prep_time_minutes}m estimated prep time</span>
                </div>
              </div>

              {/* Total & Payment */}
              <div className="p-4 rounded-2xl bg-[#F8F1E8]/70 border border-[#E9DED1]">
                <div className="text-xs font-bold text-[#81746A] flex items-center gap-1.5 mb-1">
                  <Hash className="w-3.5 h-3.5 text-[#2D663E]" />
                  <span>Payment Status</span>
                </div>
                <div className="text-base font-black text-[#33251D]">
                  ₹{order.total_amount.toFixed(2)}
                </div>
                <div className="text-[11px] text-[#81746A] mt-0.5">
                  {order.payment_status === 'PAID' ? 'Verified by Canteen Staff' : 'Pay at Counter (Cash/UPI)'}
                </div>
              </div>
            </div>

            {/* Line items table */}
            <div className="space-y-2.5 pt-1">
              <h3 className="text-xs font-bold text-[#81746A] uppercase tracking-wider">
                Line Items
              </h3>
              <div className="rounded-2xl overflow-hidden border border-[#E9DED1] bg-[#FFFDF8]">
                <table className="w-full text-left text-xs text-[#33251D]">
                  <thead className="bg-[#F8F1E8] text-[#81746A] font-bold border-b border-[#E9DED1]">
                    <tr>
                      <th className="py-2.5 px-4">Item</th>
                      <th className="py-2.5 px-4 text-center">Qty</th>
                      <th className="py-2.5 px-4 text-right">Unit Price</th>
                      <th className="py-2.5 px-4 text-right">Subtotal</th>
                    </tr>
                  </thead>
                  <tbody className="divide-y divide-[#E9DED1]/60">
                    {order.items.map((line, idx) => (
                      <tr key={idx} className="hover:bg-[#F8F1E8]/40 transition">
                        <td className="py-2.5 px-4 font-semibold text-[#33251D]">{line.item_name}</td>
                        <td className="py-2.5 px-4 text-center font-bold text-[#C95725]">{line.quantity}</td>
                        <td className="py-2.5 px-4 text-right text-[#81746A]">₹{line.price.toFixed(2)}</td>
                        <td className="py-2.5 px-4 text-right font-extrabold text-[#33251D]">
                          ₹{(line.price * line.quantity).toFixed(2)}
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};
