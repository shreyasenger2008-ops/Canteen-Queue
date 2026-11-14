import React, { useState } from 'react';
import type { CartItem, Counter, Order } from '../types';
import { placeOrder } from '../services/api';
import { X, Trash2, Minus, Plus, ShoppingCart, Loader2, AlertCircle, CreditCard } from 'lucide-react';
import { getItemImage } from './MenuSection';

interface Props {
  isOpen: boolean;
  onClose: () => void;
  cart: CartItem[];
  onUpdateQty: (id: number, delta: number) => void;
  onRemoveItem: (id: number) => void;
  onClearCart?: () => void;
  counters?: Counter[];
  onOrderSuccess: (order: Order) => void;
}

export const CartDrawer: React.FC<Props> = ({
  isOpen, onClose, cart, onUpdateQty, onRemoveItem, onOrderSuccess,
}) => {
  const [studentId,   setStudentId]   = useState('');
  const [studentName, setStudentName] = useState('');
  const [placing,     setPlacing]     = useState(false);
  const [error,       setError]       = useState<string | null>(null);

  const subtotal = cart.reduce((s, ci) => s + ci.item.price * ci.quantity, 0);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!studentId.trim() || !studentName.trim()) { setError('Student ID and name are required.'); return; }
    if (cart.length === 0) { setError('Cart is empty.'); return; }
    setError(null);
    setPlacing(true);
    try {
      const order = await placeOrder({
        student_id:      studentId.trim(),
        student_name:    studentName.trim(),
        idempotency_key: `${studentId.trim()}-${Date.now()}`,
        items: cart.map(ci => ({ item_id: ci.item.id, quantity: ci.quantity })),
      });
      onOrderSuccess(order);
      setStudentId('');
      setStudentName('');
    } catch (e: any) {
      setError(e.message || 'Failed to place order.');
    } finally {
      setPlacing(false);
    }
  };

  if (!isOpen) return null;

  return (
    <>
      <div className="fixed inset-0 z-50 bg-black/30 animate-fade-in" onClick={onClose} />
      <div
        className="fixed right-0 top-0 bottom-0 z-50 w-full max-w-md flex flex-col animate-slide-in"
        style={{ background: 'var(--surface)', boxShadow: '0 0 40px rgba(0,0,0,.12)' }}
      >
        {/* Header */}
        <div className="flex items-center justify-between px-5 py-4 border-b" style={{ borderColor: 'var(--border)' }}>
          <div className="flex items-center gap-2">
            <ShoppingCart className="w-5 h-5" style={{ color: 'var(--primary)' }} />
            <span className="font-semibold text-base" style={{ color: 'var(--text)' }}>Your Cart</span>
            {cart.length > 0 && (
              <span className="text-xs font-medium px-2 py-0.5" style={{ background: 'var(--raised)', color: 'var(--muted)', borderRadius: '6px' }}>
                {cart.reduce((s, c) => s + c.quantity, 0)} items
              </span>
            )}
          </div>
          <button
            onClick={onClose}
            className="w-8 h-8 flex items-center justify-center cursor-pointer transition"
            style={{ background: 'var(--raised)', color: 'var(--muted)', borderRadius: '8px', border: 'none' }}
          >
            <X className="w-4 h-4" />
          </button>
        </div>

        {cart.length === 0 ? (
          <div className="flex-1 flex flex-col items-center justify-center gap-4 px-6">
            <div
              className="w-16 h-16 flex items-center justify-center rounded-full"
              style={{ background: 'var(--raised)' }}
            >
              <ShoppingCart className="w-8 h-8" style={{ color: 'var(--faint)' }} />
            </div>
            <p className="font-semibold text-base" style={{ color: 'var(--text)' }}>Your cart is empty</p>
            <p className="text-sm text-center" style={{ color: 'var(--muted)' }}>Add dishes from the menu to get started.</p>
            <button
              onClick={onClose}
              className="btn-orange mt-1 px-6 py-2.5 text-sm"
            >
              Browse Menu
            </button>
          </div>
        ) : (
          <form id="order-form" onSubmit={handleSubmit} className="flex-1 overflow-y-auto flex flex-col">
            {/* Items */}
            <div className="flex-1 overflow-y-auto px-5 py-4 space-y-2">
              {cart.map(ci => {
                const imgUrl = getItemImage(ci.item.name);
                return (
                  <div
                    key={ci.item.id}
                    className="flex items-center gap-3 p-3 border"
                    style={{ background: 'var(--surface)', borderColor: 'var(--border)', borderRadius: '12px' }}
                  >
                    {imgUrl ? (
                      <img src={imgUrl} alt={ci.item.name} className="w-12 h-12 object-cover shrink-0" style={{ borderRadius: '8px' }} />
                    ) : (
                      <div className="w-12 h-12 shrink-0 flex items-center justify-center" style={{ background: 'var(--raised)', borderRadius: '8px' }}>
                        <ShoppingCart className="w-5 h-5" style={{ color: 'var(--faint)' }} />
                      </div>
                    )}
                    <div className="flex-1 min-w-0">
                      <p className="font-medium text-sm truncate" style={{ color: 'var(--text)' }}>{ci.item.name}</p>
                      <p className="text-xs num" style={{ color: 'var(--muted)' }}>₹{ci.item.price} each</p>
                    </div>
                    <div className="flex items-center gap-2">
                      <div
                        className="flex items-center overflow-hidden border"
                        style={{ borderColor: 'var(--primary)', borderRadius: '8px' }}
                      >
                        <button type="button" onClick={() => onUpdateQty(ci.item.id, -1)}
                          className="w-7 h-7 flex items-center justify-center cursor-pointer"
                          style={{ color: 'var(--primary)', background: 'var(--raised)', border: 'none' }}>
                          <Minus className="w-3 h-3" />
                        </button>
                        <span className="w-6 text-center text-xs font-semibold num" style={{ color: 'var(--text)' }}>{ci.quantity}</span>
                        <button type="button" onClick={() => onUpdateQty(ci.item.id, +1)}
                          className="w-7 h-7 flex items-center justify-center cursor-pointer"
                          style={{ color: 'var(--primary)', background: 'var(--raised)', border: 'none' }}>
                          <Plus className="w-3 h-3" />
                        </button>
                      </div>
                      <button type="button" onClick={() => onRemoveItem(ci.item.id)}
                        className="w-7 h-7 flex items-center justify-center cursor-pointer"
                        style={{ color: 'var(--faint)', background: 'none', border: 'none' }}>
                        <Trash2 className="w-3.5 h-3.5" />
                      </button>
                    </div>
                    <span className="font-semibold text-sm w-12 text-right num" style={{ color: 'var(--text)' }}>₹{(ci.item.price * ci.quantity).toFixed(0)}</span>
                  </div>
                );
              })}
            </div>

            {/* Bill */}
            <div className="mx-5 mb-4 p-4 border space-y-2" style={{ background: 'var(--raised)', borderColor: 'var(--border)', borderRadius: '12px' }}>
              <div className="flex justify-between text-sm" style={{ color: 'var(--muted)' }}>
                <span>Subtotal</span><span className="num">₹{subtotal.toFixed(2)}</span>
              </div>
              <div className="flex justify-between font-semibold text-base border-t pt-2" style={{ borderColor: 'var(--border)', color: 'var(--primary)' }}>
                <span style={{ color: 'var(--text)' }}>Total</span><span className="num">₹{subtotal.toFixed(2)}</span>
              </div>
              <div className="flex items-center gap-1.5 pt-1" style={{ color: 'var(--muted)' }}>
                <CreditCard className="w-3.5 h-3.5 shrink-0" />
                <p className="text-xs">Pay cash or UPI at the counter when you collect</p>
              </div>
            </div>

            {/* Student info */}
            <div className="px-5 pb-3 space-y-2">
              <p className="text-xs font-medium" style={{ color: 'var(--muted)' }}>Your details</p>
              <input type="text" placeholder="Student ID (e.g. STU101)" value={studentId} onChange={e => setStudentId(e.target.value)} required
                className="w-full px-3 text-sm border outline-none transition"
                style={{ background: 'var(--surface)', borderColor: 'var(--border)', color: 'var(--text)', borderRadius: '8px', height: '42px' }} />
              <input type="text" placeholder="Your name (e.g. Rahul Sharma)" value={studentName} onChange={e => setStudentName(e.target.value)} required
                className="w-full px-3 text-sm border outline-none transition"
                style={{ background: 'var(--surface)', borderColor: 'var(--border)', color: 'var(--text)', borderRadius: '8px', height: '42px' }} />
            </div>

            {error && (
              <div className="mx-5 mb-3 flex items-center gap-2 text-xs px-3 py-2.5" style={{ background: 'var(--pill-negative-bg)', color: 'var(--pill-negative-ink)', borderRadius: '8px' }}>
                <AlertCircle className="w-3.5 h-3.5 shrink-0" />{error}
              </div>
            )}

            {/* Place order */}
            <div className="px-5 pb-5">
              <button type="submit" form="order-form" disabled={placing || cart.length === 0}
                className="w-full font-semibold text-sm text-white flex items-center justify-center gap-2 transition disabled:opacity-60 cursor-pointer"
                style={{ background: 'var(--primary)', borderRadius: '12px', height: '48px', border: 'none' }}>
                {placing ? <><Loader2 className="w-4 h-4 animate-spin" />Placing order…</> : <>Place Order · ₹{subtotal.toFixed(0)}</>}
              </button>
            </div>
          </form>
        )}
      </div>
    </>
  );
};
