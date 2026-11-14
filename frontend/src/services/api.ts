import type {
  MenuItem,
  Counter,
  Order,
  CreateOrderRequest,
  AnalyticsData,
  HealthStatus,
  PaginatedOrdersResponse,
} from '../types';

const API_BASE_URL = (import.meta.env.VITE_API_BASE_URL || '').replace(/\/+$/, '');

// ---------------------------------------------------------------------------
// Staff token management
// Token is obtained from the server via POST /api/staff/auth and stored in
// sessionStorage. It is NEVER hardcoded here; it comes from the C++ backend
// which reads CANTEEN_STAFF_TOKEN from the environment.
// ---------------------------------------------------------------------------
const STAFF_TOKEN_KEY = 'canteen_staff_token';

export function getStaffToken(): string | null {
  return sessionStorage.getItem(STAFF_TOKEN_KEY);
}

export function setStaffToken(token: string): void {
  sessionStorage.setItem(STAFF_TOKEN_KEY, token);
}

export function clearStaffToken(): void {
  sessionStorage.removeItem(STAFF_TOKEN_KEY);
  sessionStorage.removeItem('canteen_staff_auth');
}

export class ApiError extends Error {
  status: number;
  data?: unknown;

  constructor(message: string, status: number, data?: unknown) {
    super(message);
    this.name = 'ApiError';
    this.status = status;
    this.data = data;
  }
}

async function handleResponse<T>(res: Response): Promise<T> {
  const contentType = res.headers.get('content-type') || '';
  const isJson = contentType.includes('application/json');

  let bodyData: any = null;
  if (isJson) {
    try {
      bodyData = await res.json();
    } catch {
      bodyData = null;
    }
  } else {
    try {
      bodyData = await res.text();
    } catch {
      bodyData = null;
    }
  }

  if (!res.ok) {
    let errorMessage = `HTTP Error ${res.status}: ${res.statusText}`;
    if (bodyData && typeof bodyData === 'object' && bodyData.error) {
      errorMessage = bodyData.error;
    } else if (typeof bodyData === 'string' && bodyData.length > 0) {
      errorMessage = bodyData;
    }
    throw new ApiError(errorMessage, res.status, bodyData);
  }

  return bodyData as T;
}

/** Base request — no auth header (student-facing endpoints). */
async function request<T>(endpoint: string, options: RequestInit = {}): Promise<T> {
  const url = `${API_BASE_URL}${endpoint}`;
  try {
    const res = await fetch(url, {
      ...options,
      headers: {
        'Content-Type': 'application/json',
        ...(options.headers || {}),
      },
    });
    return await handleResponse<T>(res);
  } catch (err: any) {
    if (err instanceof ApiError) {
      throw err;
    }
    const msg = `Unable to reach canteen backend. Ensure the C++ server is running: run 'build\\Debug\\canteen_app.exe --server 8080' in the project folder.`;
    throw new ApiError(msg, 0);
  }
}

/** Staff request — attaches the Bearer token from sessionStorage. */
async function staffRequest<T>(endpoint: string, options: RequestInit = {}): Promise<T> {
  const token = getStaffToken();
  const authHeader: Record<string, string> = token
    ? { Authorization: `Bearer ${token}` }
    : {};

  return request<T>(endpoint, {
    ...options,
    headers: {
      ...authHeader,
      ...(options.headers || {}),
    },
  });
}

// ---------------------------------------------------------------------------
// Staff Authentication
// ---------------------------------------------------------------------------

/** POST /api/staff/auth — exchange PIN for server-side token. */
export async function staffLogin(pin: string): Promise<{ token: string; message: string }> {
  const result = await request<{ token: string; message: string }>('/api/staff/auth', {
    method: 'POST',
    body: JSON.stringify({ pin }),
  });
  // Persist token in sessionStorage for subsequent staff requests
  setStaffToken(result.token);
  sessionStorage.setItem('canteen_staff_auth', 'true');
  return result;
}

// ---------------------------------------------------------------------------
// Student-facing endpoints (no auth required)
// ---------------------------------------------------------------------------

// 1. GET /api/health
export async function getHealth(): Promise<HealthStatus> {
  return request<HealthStatus>('/api/health');
}

// 2. GET /api/menu
export async function getMenu(): Promise<MenuItem[]> {
  return request<MenuItem[]>('/api/menu');
}

// 3. GET /api/counters
export async function getCounters(): Promise<Counter[]> {
  return request<Counter[]>('/api/counters');
}

// 4. POST /api/orders
export async function placeOrder(orderReq: CreateOrderRequest): Promise<Order> {
  return request<Order>('/api/orders', {
    method: 'POST',
    body: JSON.stringify(orderReq),
  });
}

// 4b. GET /api/orders (paginated list with optional status filter)
export async function getOrders(params?: {
  limit?: number;
  offset?: number;
  status?: string;
}): Promise<PaginatedOrdersResponse> {
  const searchParams = new URLSearchParams();
  if (params?.limit !== undefined) searchParams.set('limit', String(params.limit));
  if (params?.offset !== undefined) searchParams.set('offset', String(params.offset));
  if (params?.status && params.status !== 'ALL') searchParams.set('status', params.status);

  const qs = searchParams.toString();
  return request<PaginatedOrdersResponse>(`/api/orders${qs ? `?${qs}` : ''}`);
}

// 5. GET /api/orders/{id}  (student tracking — no auth)
export async function getOrderById(orderId: number): Promise<Order> {
  return request<Order>(`/api/orders/${orderId}`);
}

// 10. GET /api/analytics
export async function getAnalytics(): Promise<AnalyticsData> {
  return request<AnalyticsData>('/api/analytics');
}

// ---------------------------------------------------------------------------
// Staff-only endpoints (require Bearer token from staffLogin)
// ---------------------------------------------------------------------------

// 6. POST /api/orders/{id}/start
export async function startOrder(orderId: number): Promise<Order> {
  return staffRequest<Order>(`/api/orders/${orderId}/start`, { method: 'POST' });
}

// 7. POST /api/orders/{id}/ready
export async function readyOrder(orderId: number): Promise<Order> {
  return staffRequest<Order>(`/api/orders/${orderId}/ready`, { method: 'POST' });
}

// 8. POST /api/orders/{id}/collect
export async function collectOrder(orderId: number): Promise<Order> {
  return staffRequest<Order>(`/api/orders/${orderId}/collect`, { method: 'POST' });
}

// 9. POST /api/orders/{id}/cancel
export async function cancelOrder(orderId: number): Promise<Order> {
  return staffRequest<Order>(`/api/orders/${orderId}/cancel`, { method: 'POST' });
}

// 9b. POST /api/orders/{id}/pay (Confirm counter payment — staff only)
export async function payOrder(orderId: number): Promise<Order> {
  return staffRequest<Order>(`/api/orders/${orderId}/pay`, { method: 'POST' });
}

// 9c. POST /api/orders/{id}/accept (Acknowledge incoming order — staff only)
export async function acceptOrder(orderId: number): Promise<Order> {
  return staffRequest<Order>(`/api/orders/${orderId}/accept`, { method: 'POST' });
}

// 9d. DELETE /api/orders/{id} (Permanently remove order — staff only)
export async function deleteOrder(orderId: number): Promise<{ deleted: boolean; id: number; message: string }> {
  return staffRequest<{ deleted: boolean; id: number; message: string }>(`/api/orders/${orderId}`, {
    method: 'DELETE',
  });
}

export function getApiBaseUrl(): string {
  return API_BASE_URL;
}
