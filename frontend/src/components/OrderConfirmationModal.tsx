import React from 'react';
import { CheckCircle2, Clock, MapPin, Hash, Sparkles, ArrowRight, X, CreditCard } from 'lucide-react';
import type { Order, Counter } from '../types';
import { StatusBadge, PaymentBadge } from './StatusBadge';

interface Props {
  order: Order | null;
  onClose: () => void;
  onTrackOrder: (orderId: number) => void;
  counters: Counter[];
}

export const OrderConfirmationModal: React.FC<Props> = ({
  order,
  onClose,
  onTrackOrder,
  counters,
}) => {
  if (!order) return null;

  const assignedCounter = counters.find((c) => c.id === order.assigned_counter_id);

  return (
    <div className="fixed inset-0 z-50 overflow-y-auto flex items-center justify-center p-4 bg-[#33251D]/60 backdrop-blur-xs animate-fadeIn">
      <div className="w-full max-w-md bg-[#FFFDF8] border border-[#E9DED1] rounded-3xl shadow-xl p-6 sm:p-8 relative overflow-hidden">
        {/* Decorative terracotta top bar */}
        <div className="absolute top-0 left-0 right-0 h-2 bg-[#C95725]" />

        <button
          onClick={onClose}
          className="absolute top-4 right-4 p-2 rounded-xl text-[#81746A] hover:text-[#33251D] hover:bg-[#F8F1E8] transition"
          aria-label="Close modal"
        >
          <X className="w-5 h-5" />
        </button>

        {/* Success Icon */}
        <div className="text-center pt-2">
          <div className="w-16 h-16 mx-auto rounded-2xl bg-[#EFF8F1] border border-[#CFEAD5] flex items-center justify-center mb-3">
            <CheckCircle2 className="w-9 h-9 text-[#2D663E]" />
          </div>
          <h3 className="font-serif text-2xl font-bold text-[#33251D] tracking-tight">Order Confirmed!</h3>
          <p className="text-xs text-[#81746A] mt-1 font-medium">
            Assigned to optimal counter queue via C++ Random Forest Scheduler
          </p>
        </div>

        {/* Order Details Card */}
        <div className="mt-5 p-4 rounded-2xl bg-[#F8F1E8]/60 border border-[#E9DED1] space-y-3">
          <div className="flex items-center justify-between pb-2.5 border-b border-[#E9DED1]">
            <div className="flex items-center gap-1.5 text-xs text-[#81746A] font-semibold">
              <Hash className="w-4 h-4 text-[#81746A]" />
              <span>Order Number</span>
            </div>
            <span className="text-lg font-black text-[#33251D]">
              #{order.id}
            </span>
          </div>

          <div className="flex items-center justify-between pb-2.5 border-b border-[#E9DED1]">
            <div className="flex items-center gap-1.5 text-xs text-[#81746A] font-semibold">
              <MapPin className="w-4 h-4 text-[#C95725]" />
              <span>Assigned Counter</span>
            </div>
            <span className="text-sm font-bold text-[#33251D]">
              {assignedCounter ? assignedCounter.name : `Counter #${order.assigned_counter_id}`}
            </span>
          </div>

          <div className="flex items-center justify-between pb-2.5 border-b border-[#E9DED1]">
            <div className="flex items-center gap-1.5 text-xs text-[#81746A] font-semibold">
              <Clock className="w-4 h-4 text-[#C95725]" />
              <span>Predicted Prep Time</span>
            </div>
            <span className="inline-flex items-center gap-1 text-xs font-bold text-[#C95725] px-2 py-0.5 rounded-md bg-[#FDF0E9] border border-[#F4D1BD]">
              <Sparkles className="w-3 h-3 text-[#C95725]" />
              ~{order.total_prep_time_minutes} minutes
            </span>
          </div>

          <div className="flex items-center justify-between pb-2.5 border-b border-[#E9DED1]">
            <span className="text-xs text-[#81746A] font-semibold">Payment Status</span>
            <PaymentBadge status={order.payment_status} size="sm" />
          </div>

          <div className="flex items-center justify-between">
            <span className="text-xs text-[#81746A] font-semibold">Current Queue State</span>
            <StatusBadge status={order.status} size="sm" />
          </div>
        </div>

        {/* Clear Next Steps for Student */}
        <div className="mt-4 p-3 rounded-xl bg-[#FFFDF8] border border-[#E9DED1] text-[11px] text-[#81746A] space-y-1">
          <div className="font-bold text-[#33251D] flex items-center gap-1">
            <CreditCard className="w-3.5 h-3.5 text-[#C95725]" />
            <span>Next Steps:</span>
          </div>
          <p>
            1. Track your live queue position on the tracking screen.
          </p>
          <p>
            2. When marked <strong>Ready for Pickup</strong>, proceed to <strong>{assignedCounter?.name || `Counter #${order.assigned_counter_id}`}</strong>.
          </p>
          <p>
            3. Show Order #{order.id} and complete counter payment (Cash/UPI) to collect your meal.
          </p>
        </div>

        {/* Items Summary */}
        <div className="mt-4 text-xs">
          <div className="flex justify-between items-center py-1 font-bold text-[#33251D]">
            <span>Total Amount to Pay</span>
            <span className="text-base text-[#C95725]">₹{order.total_amount.toFixed(2)}</span>
          </div>
        </div>

        {/* Actions */}
        <div className="mt-5 flex items-center gap-3">
          <button
            onClick={() => {
              onClose();
              onTrackOrder(order.id);
            }}
            className="flex-1 py-3 px-4 rounded-xl bg-[#C95725] hover:bg-[#B3471A] text-white font-bold text-sm flex items-center justify-center gap-2 transition shadow-xs cursor-pointer"
          >
            <span>Live Order Tracking</span>
            <ArrowRight className="w-4 h-4" />
          </button>
          <button
            onClick={onClose}
            className="py-3 px-4 rounded-xl bg-[#F8F1E8] hover:bg-[#E9DED1] text-[#33251D] font-bold text-sm transition cursor-pointer border border-[#E9DED1]"
          >
            Done
          </button>
        </div>
      </div>
    </div>
  );
};
