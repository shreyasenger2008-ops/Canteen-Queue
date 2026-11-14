import React, { useState } from 'react';
import {
  X,
  Trash2,
  Plus,
  Minus,
  Sparkles,
  ArrowRight,
  AlertCircle,
  Clock,
  Loader2,
  Receipt,
  UserCheck,
  CreditCard,
} from 'lucide-react';
import type { CartItem, Order, Counter } from '../types';
import { placeOrder } from '../services/api';
import { generateIdempotencyKey, saveRecentOrder } from '../utils/storage';

interface Props {
  isOpen: boolean;
  onClose: () => void;
  items: CartItem[];
  onUpdateQuantity: (itemId: number, delta: number) => void;
  onRemoveItem: (itemId: number) => void;
  onClearCart: () => void;
  onOrderSuccess: (order: Order) => void;
  counters: Counter[];
}

export const CartDrawer: React.FC<Props> = ({
  isOpen,
  onClose,
  items,
  onUpdateQuantity,
  onRemoveItem,
  onClearCart,
  onOrderSuccess,
  counters,
}) => {
  const [studentId, setStudentId] = useState('');
  const [studentName, setStudentName] = useState('');
  const [preferredCounterId, setPreferredCounterId] = useState<number>(-1);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  if (!isOpen) return null;

  const totalAmount = items.reduce(
    (sum, ci) => sum + ci.item.price * ci.quantity,
    0
  );
  const totalItemsCount = items.reduce((sum, ci) => sum + ci.quantity, 0);

  const handleSubmitOrder = async (e: React.FormEvent) => {
    e.preventDefault();
    setError(null);

    if (!studentId.trim()) {
      setError('Student ID is required (e.g. STU101).');
      return;
    }
    if (!studentName.trim()) {
      setError('Student Name is required.');
      return;
    }
    if (items.length === 0) {
      setError('Your cart is empty.');
      return;
    }

    setLoading(true);
    try {
      const idempotencyKey = generateIdempotencyKey();
      const orderPayload = {
        student_id: studentId.trim(),
        student_name: studentName.trim(),
        idempotency_key: idempotencyKey,
        preferred_counter_id: preferredCounterId,
        items: items.map((ci) => ({
          item_id: ci.item.id,
          quantity: ci.quantity,
        })),
      };

      const placed = await placeOrder(orderPayload);
      saveRecentOrder(placed);
      onClearCart();
      onClose();
      onOrderSuccess(placed);
    } catch (err: any) {
      console.error('Order placement failed', err);
      setError(err.message || 'Failed to submit order to C++ backend.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="fixed inset-0 z-50 overflow-hidden">
      {/* Backdrop */}
      <div
        className="absolute inset-0 bg-[#33251D]/60 backdrop-blur-xs transition-opacity"
        onClick={onClose}
      />

      <div className="fixed inset-y-0 right-0 max-w-full flex pl-10">
        <div className="w-screen max-w-md bg-[#FFFDF8] border-l border-[#E9DED1] shadow-2xl flex flex-col">
          {/* Header */}
          <div className="p-5 border-b border-[#E9DED1] flex items-center justify-between bg-[#FFFDF8]">
            <div className="flex items-center gap-2.5">
              <h2 className="font-serif text-lg font-bold text-[#33251D] tracking-tight">Your Order</h2>
              <span className="text-xs font-bold px-2 py-0.5 rounded-full bg-[#FDF0E9] text-[#C95725] border border-[#F4D1BD]">
                {totalItemsCount} {totalItemsCount === 1 ? 'item' : 'items'}
              </span>
            </div>
            <button
              onClick={onClose}
              className="p-1.5 rounded-xl text-[#81746A] hover:text-[#33251D] hover:bg-[#F8F1E8] transition"
              aria-label="Close cart"
            >
              <X className="w-5 h-5" />
            </button>
          </div>

          {/* Cart Items List & Verification Form */}
          <div className="flex-1 overflow-y-auto p-5 space-y-5 bg-[#F8F1E8]/50">
            {error && (
              <div className="p-3.5 rounded-xl bg-[#FDF0F0] border border-[#F4CECE] text-[#9E3939] text-xs flex items-start gap-2.5">
                <AlertCircle className="w-4 h-4 text-[#9E3939] shrink-0 mt-0.5" />
                <span>{error}</span>
              </div>
            )}

            {items.length === 0 ? (
              <div className="h-48 flex flex-col items-center justify-center text-center text-[#81746A] bg-[#FFFDF8] rounded-2xl border border-[#E9DED1] p-6">
                <p className="text-sm font-bold text-[#33251D]">Your cart is empty.</p>
                <p className="text-xs text-[#81746A] mt-1">
                  Add freshly prepared dishes from the canteen menu.
                </p>
              </div>
            ) : (
              <div className="space-y-3">
                <div className="text-xs font-bold uppercase tracking-wider text-[#81746A]">
                  Selected Dishes
                </div>
                {items.map(({ item, quantity }) => (
                  <div
                    key={item.id}
                    className="p-3.5 rounded-2xl bg-[#FFFDF8] border border-[#E9DED1] shadow-xs flex items-center justify-between gap-3"
                  >
                    <div className="flex items-start gap-2.5 min-w-0 flex-1">
                      {/* Veg indicator square */}
                      <div className="mt-1 w-3.5 h-3.5 border border-emerald-600 flex items-center justify-center rounded-[3px] shrink-0">
                        <div className="w-1.5 h-1.5 rounded-full bg-emerald-600" />
                      </div>

                      <div className="min-w-0 flex-1">
                        <div className="font-bold text-sm text-[#33251D] truncate">
                          {item.name}
                        </div>
                        <div className="flex items-center gap-2 text-xs text-[#81746A] mt-0.5">
                          <span className="text-[#33251D] font-extrabold">
                            ₹{item.price.toFixed(2)}
                          </span>
                          <span>•</span>
                          <span className="flex items-center gap-1">
                            <Clock className="w-3 h-3 text-[#81746A]" />
                            ~{item.prep_time_minutes}m
                          </span>
                        </div>
                      </div>
                    </div>

                    {/* Quantity Selector */}
                    <div className="flex items-center gap-2 bg-[#FDF0E9] border border-[#F4D1BD] px-2 py-1 rounded-xl">
                      <button
                        type="button"
                        onClick={() => onUpdateQuantity(item.id, -1)}
                        className="text-[#C95725] hover:text-[#B3471A] p-0.5 rounded transition"
                        aria-label="Decrease quantity"
                      >
                        <Minus className="w-3.5 h-3.5" />
                      </button>
                      <span className="text-xs font-black text-[#C95725] min-w-[1.2rem] text-center">
                        {quantity}
                      </span>
                      <button
                        type="button"
                        onClick={() => onUpdateQuantity(item.id, 1)}
                        className="text-[#C95725] hover:text-[#B3471A] p-0.5 rounded transition"
                        aria-label="Increase quantity"
                      >
                        <Plus className="w-3.5 h-3.5" />
                      </button>
                    </div>

                    <button
                      type="button"
                      onClick={() => onRemoveItem(item.id)}
                      className="p-1.5 text-[#81746A] hover:text-[#C95725] transition"
                      title="Remove item"
                    >
                      <Trash2 className="w-4 h-4" />
                    </button>
                  </div>
                ))}
              </div>
            )}

            {/* Student Details Form */}
            {items.length > 0 && (
              <form
                id="order-form"
                onSubmit={handleSubmitOrder}
                className="p-4 rounded-2xl bg-[#FFFDF8] border border-[#E9DED1] shadow-xs space-y-4"
              >
                <div className="flex items-center gap-2 text-xs font-bold uppercase tracking-wider text-[#33251D]">
                  <UserCheck className="w-4 h-4 text-[#C95725]" />
                  <span>Student Verification</span>
                </div>

                <div>
                  <label className="block text-xs font-bold text-[#33251D] mb-1">
                    Student ID <span className="text-[#C95725]">*</span>
                  </label>
                  <input
                    type="text"
                    required
                    placeholder="e.g. STU101 or 2024CS042"
                    value={studentId}
                    onChange={(e) => setStudentId(e.target.value)}
                    className="w-full px-3.5 py-2.5 rounded-xl bg-[#F8F1E8]/50 border border-[#E9DED1] text-sm text-[#33251D] placeholder-[#81746A] focus:outline-none focus:border-[#C95725] focus:bg-[#FFFDF8] transition"
                  />
                </div>

                <div>
                  <label className="block text-xs font-bold text-[#33251D] mb-1">
                    Student Name <span className="text-[#C95725]">*</span>
                  </label>
                  <input
                    type="text"
                    required
                    placeholder="e.g. Rahul Sharma"
                    value={studentName}
                    onChange={(e) => setStudentName(e.target.value)}
                    className="w-full px-3.5 py-2.5 rounded-xl bg-[#F8F1E8]/50 border border-[#E9DED1] text-sm text-[#33251D] placeholder-[#81746A] focus:outline-none focus:border-[#C95725] focus:bg-[#FFFDF8] transition"
                  />
                </div>

                <div>
                  <label className="block text-xs font-bold text-[#33251D] mb-1">
                    Preferred Counter
                  </label>
                  <select
                    value={preferredCounterId}
                    onChange={(e) => setPreferredCounterId(Number(e.target.value))}
                    className="w-full px-3.5 py-2.5 rounded-xl bg-[#F8F1E8]/50 border border-[#E9DED1] text-sm text-[#33251D] focus:outline-none focus:border-[#C95725] focus:bg-[#FFFDF8] transition"
                  >
                    <option value={-1}>✨ Auto AI Load Balanced (Recommended)</option>
                    {counters.map((c) => (
                      <option key={c.id} value={c.id} disabled={!c.is_available}>
                        Counter #{c.id}: {c.name} ({c.pending_orders_count} queued
                        {!c.is_available ? ' - OFFLINE' : ''})
                      </option>
                    ))}
                  </select>
                </div>

                {/* Honest Payment Workflow Notice */}
                <div className="p-3 rounded-xl bg-[#FDF6E9] border border-[#EAD8BA] text-[11px] text-[#8C5D1F] flex items-start gap-2">
                  <CreditCard className="w-4 h-4 text-[#8C5D1F] shrink-0 mt-0.5" />
                  <div>
                    <span className="font-bold">Payment Method: Pay at Counter</span>
                    <p className="mt-0.5 text-[10px] leading-relaxed">
                      Cash or UPI accepted when picking up your food. Staff will confirm payment at the counter.
                    </p>
                  </div>
                </div>

                <div className="p-3 rounded-xl bg-[#FDF0E9] border border-[#F4D1BD] text-[11px] text-[#C95725] flex items-start gap-2">
                  <Sparkles className="w-4 h-4 text-[#C95725] shrink-0 mt-0.5" />
                  <p>
                    Orders are automatically balanced across canteen queues via our trained <strong>Random Forest Regressor</strong> model.
                  </p>
                </div>
              </form>
            )}

            {/* Bill Details Breakdown */}
            {items.length > 0 && (
              <div className="p-4 rounded-2xl bg-[#FFFDF8] border border-[#E9DED1] shadow-xs space-y-2.5 text-xs">
                <div className="flex items-center gap-1.5 font-bold uppercase tracking-wider text-[#33251D] mb-1">
                  <Receipt className="w-3.5 h-3.5 text-[#81746A]" />
                  <span>Bill Details</span>
                </div>

                <div className="flex justify-between text-[#81746A]">
                  <span>Item Total</span>
                  <span className="font-semibold text-[#33251D]">₹{totalAmount.toFixed(2)}</span>
                </div>

                <div className="flex justify-between text-[#81746A]">
                  <span>Canteen Packaging / Service</span>
                  <span className="font-semibold text-emerald-600 uppercase text-[11px]">Free</span>
                </div>

                <div className="flex justify-between text-[#81746A]">
                  <span>Taxes & Canteen Cess</span>
                  <span className="font-semibold text-[#33251D]">₹0.00</span>
                </div>

                <div className="pt-2 border-t border-[#E9DED1] flex justify-between items-center text-sm font-bold text-[#33251D]">
                  <span>To Pay at Counter</span>
                  <span className="font-extrabold">₹{totalAmount.toFixed(2)}</span>
                </div>
              </div>
            )}
          </div>

          {/* Footer & Submit Button */}
          {items.length > 0 && (
            <div className="p-5 border-t border-[#E9DED1] bg-[#FFFDF8] space-y-3">
              <div className="flex items-center justify-between text-sm">
                <span className="font-bold text-[#81746A]">Grand Total:</span>
                <span
                  className="text-xl font-extrabold text-amber-400"
                  style={{ color: '#33251D' }}
                >
                  ₹{totalAmount.toFixed(2)}
                </span>
              </div>

              <button
                type="submit"
                form="order-form"
                disabled={loading}
                className="w-full py-3.5 px-4 rounded-xl bg-[#C95725] hover:bg-[#B3471A] text-white font-bold text-sm flex items-center justify-center gap-2 shadow-xs transition disabled:opacity-50 disabled:cursor-not-allowed cursor-pointer"
              >
                {loading ? (
                  <>
                    <Loader2 className="w-4 h-4 animate-spin" />
                    <span>Scheduling via C++ AI...</span>
                  </>
                ) : (
                  <>
                    <span>Confirm & Place Order</span>
                    <ArrowRight className="w-4 h-4" />
                  </>
                )}
              </button>
            </div>
          )}
        </div>
      </div>
    </div>
  );
};
