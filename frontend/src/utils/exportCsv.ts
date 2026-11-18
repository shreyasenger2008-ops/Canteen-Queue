import type { Order } from '../types';

function escapeCsvCell(value: string | number | null | undefined): string {
  const str = String(value ?? '');
  // If the cell contains a comma, double-quote, or newline, wrap in double quotes and escape inner quotes
  if (str.includes(',') || str.includes('"') || str.includes('\n')) {
    return `"${str.replace(/"/g, '""')}"`;
  }
  return str;
}

export function exportOrdersToCsv(orders: Order[], filename?: string): void {
  const today = new Date().toISOString().slice(0, 10); // YYYY-MM-DD
  const outputFilename = filename ?? `canteen-orders-${today}.csv`;

  const headers = [
    'Order ID',
    'Student Name',
    'Student ID',
    'Counter',
    'Status',
    'Payment',
    'Items',
    'Total (₹)',
    'Prep Time (min)',
    'Placed At',
  ];

  const rows = orders.map((order) => {
    const items = order.items
      .map((it) => `${it.item_name} x${it.quantity}`)
      .join('; ');

    // order_timestamp is a Unix timestamp (seconds or ms — handle both)
    const ts = order.order_timestamp > 1e10
      ? order.order_timestamp          // already ms
      : order.order_timestamp * 1000;  // convert s → ms
    const placedAt = new Date(ts).toLocaleString();

    return [
      order.id,
      order.student_name,
      order.student_id,
      order.assigned_counter_id,
      order.status,
      order.payment_status ?? '',
      items,
      order.total_amount,
      order.total_prep_time_minutes,
      placedAt,
    ].map(escapeCsvCell).join(',');
  });

  const csvContent = [headers.map(escapeCsvCell).join(','), ...rows].join('\r\n');

  const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
  const url = URL.createObjectURL(blob);
  const link = document.createElement('a');
  link.href = url;
  link.setAttribute('download', outputFilename);
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
  URL.revokeObjectURL(url);
}
