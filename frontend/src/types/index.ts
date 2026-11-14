export type OrderStatus = 'QUEUED' | 'PREPARING' | 'READY' | 'COLLECTED' | 'CANCELLED';

export interface MenuItem {
  id: number;
  name: string;
  category: string;
  price: number;
  prep_time_minutes: number;
  is_available: boolean;
  complexity: number;
}

export interface Counter {
  id: number;
  name: string;
  counter_type: string;
  is_available: boolean;
  pending_orders_count: number;
  current_workload_minutes: number;
  total_orders_served: number;
}

export interface OrderLineItem {
  item_id: number;
  item_name: string;
  price: number;
  quantity: number;
}

export type PaymentStatus = 'PENDING' | 'PAID' | 'REFUNDED';

export interface Order {
  id: number;
  student_id: string;
  student_name: string;
  status: OrderStatus;
  payment_status?: PaymentStatus | string;
  queue_position?: number;
  assigned_counter_id: number;
  total_prep_time_minutes: number;
  total_amount: number;
  order_timestamp: number;
  items: OrderLineItem[];
}

export interface PaginatedOrdersResponse {
  orders: Order[];
  total: number;
  limit: number;
  offset: number;
}

export interface CreateOrderItemRequest {
  item_id: number;
  quantity: number;
}

export interface CreateOrderRequest {
  student_id: string;
  student_name: string;
  idempotency_key?: string;
  preferred_counter_id?: number;
  items: CreateOrderItemRequest[];
}

export interface AnalyticsData {
  timestamp: number;
  total_orders_history_count: number;
  currently_pending_orders: number;
  total_active_workload_minutes: number;
  ml_model_active: boolean;
  ml_model_name: string;
  counters_analytics: Counter[];
}

export interface HealthStatus {
  status: string;
  service: string;
  database_connected: boolean;
  ml_model_loaded: boolean;
  ml_model_source: string;
  active_counters_count: number;
}

export interface CartItem {
  item: MenuItem;
  quantity: number;
}

export interface RecentOrderMeta {
  id: number;
  student_name: string;
  timestamp: number;
  total_amount: number;
  status: OrderStatus;
  assigned_counter_id: number;
}
