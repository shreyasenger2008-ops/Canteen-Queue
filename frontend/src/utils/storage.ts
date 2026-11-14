import type { Order, RecentOrderMeta } from '../types';

const RECENT_ORDERS_KEY = 'canteen_recent_orders';

export function generateIdempotencyKey(): string {
  if (typeof crypto !== 'undefined' && crypto.randomUUID) {
    return crypto.randomUUID();
  }
  return 'req-' + Date.now() + '-' + Math.random().toString(36).substring(2, 9);
}

export function getRecentOrders(): RecentOrderMeta[] {
  try {
    const raw = localStorage.getItem(RECENT_ORDERS_KEY);
    if (!raw) return [];
    return JSON.parse(raw);
  } catch (err) {
    console.error('Failed to load recent orders from storage', err);
    return [];
  }
}

export function saveRecentOrder(order: Order): void {
  try {
    const existing = getRecentOrders();
    const meta: RecentOrderMeta = {
      id: order.id,
      student_name: order.student_name,
      timestamp: order.order_timestamp,
      total_amount: order.total_amount,
      status: order.status,
      assigned_counter_id: order.assigned_counter_id,
    };
    // Keep most recent first, filter out previous entry with same id
    const filtered = existing.filter((o) => o.id !== order.id);
    const updated = [meta, ...filtered].slice(0, 20); // Keep last 20
    localStorage.setItem(RECENT_ORDERS_KEY, JSON.stringify(updated));
  } catch (err) {
    console.error('Failed to save recent order', err);
  }
}

export function updateRecentOrderStatus(orderId: number, newStatus: Order['status']): void {
  try {
    const existing = getRecentOrders();
    const updated = existing.map((o) =>
      o.id === orderId ? { ...o, status: newStatus } : o
    );
    localStorage.setItem(RECENT_ORDERS_KEY, JSON.stringify(updated));
  } catch (err) {
    console.error('Failed to update recent order status', err);
  }
}
