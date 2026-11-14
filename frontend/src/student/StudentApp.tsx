/* Student App — full warm cream+orange layout */
import React, { useState } from 'react';
import type { MenuItem, CartItem, Order, Counter, HealthStatus } from '../types';
import { StudentHeader } from './StudentHeader';
import { HeroSection }   from './HeroSection';
import { MenuSection }   from './MenuSection';
import { CartDrawer }    from './CartDrawer';
import { OrderConfirmModal } from './OrderConfirmModal';
import { TrackSection }  from './TrackSection';
import { StudentFooter } from './StudentFooter';
import { QueueDashboard } from '../pages/QueueDashboard';
import { ShoppingCart } from 'lucide-react';

interface Props {
  menu: MenuItem[];
  counters: Counter[];
  health: HealthStatus | null;
  loading: boolean;
  menuError: string | null;
  onRetry: () => void;
  cart: CartItem[];
  onAddToCart: (item: MenuItem) => void;
  onUpdateQty:  (id: number, delta: number) => void;
  onRemoveItem: (id: number) => void;
  onClearCart:  () => void;
  confirmedOrder: Order | null;
  onOrderConfirmed: (order: Order) => void;
  onClearConfirmed: () => void;
  onGoStaff: () => void;
}

export type StudentView = 'home' | 'menu' | 'track' | 'queues';

export const StudentApp: React.FC<Props> = ({
  menu, counters, health, loading, menuError, onRetry,
  cart, onAddToCart, onUpdateQty, onRemoveItem, onClearCart,
  confirmedOrder, onOrderConfirmed, onClearConfirmed,
  onGoStaff,
}) => {
  const [view, setView]             = useState<StudentView>('home');
  const [isCartOpen, setCartOpen]   = useState(false);
  const [trackOrderId, setTrackId]  = useState<number | null>(null);

  const cartCount = cart.reduce((s, ci) => s + ci.quantity, 0);
  const cartTotal = cart.reduce((s, ci) => s + ci.item.price * ci.quantity, 0);

  const goTrack = (id: number) => {
    setTrackId(id);
    setView('track');
    onClearConfirmed();
  };

  return (
    <div className="min-h-screen" style={{ background: 'var(--ground)' }}>
      <StudentHeader
        view={view}
        onNav={setView}
        cartCount={cartCount}
        onOpenCart={() => setCartOpen(true)}
        health={health}
        onGoStaff={onGoStaff}
      />

      <main>
        {view === 'home' && (
          <>
            <HeroSection onOrder={() => setView('menu')} onTrack={() => setView('track')} />
            <MenuSection
              menu={menu}
              loading={loading}
              error={menuError}
              onRetry={onRetry}
              cart={cart}
              onAdd={onAddToCart}
              onUpdateQty={onUpdateQty}
              counters={counters}
              compact
              onOpenCart={() => setCartOpen(true)}
              onViewAll={() => setView('menu')}
            />
          </>
        )}

        {view === 'menu' && (
          <div className="pt-4 pb-20">
            <MenuSection
              menu={menu}
              loading={loading}
              error={menuError}
              onRetry={onRetry}
              cart={cart}
              onAdd={onAddToCart}
              onUpdateQty={onUpdateQty}
              counters={counters}
              compact={false}
              onOpenCart={() => setCartOpen(true)}
              onViewAll={() => setView('menu')}
            />
          </div>
        )}

        {view === 'track' && (
          <TrackSection initialId={trackOrderId} counters={counters} />
        )}

        {view === 'queues' && (
          <div className="py-8">
            <QueueDashboard health={health} />
          </div>
        )}
      </main>

      <StudentFooter />

      {/* Cart Drawer */}
      <CartDrawer
        isOpen={isCartOpen}
        onClose={() => setCartOpen(false)}
        cart={cart}
        onUpdateQty={onUpdateQty}
        onRemoveItem={onRemoveItem}
        onClearCart={onClearCart}
        counters={counters}
        onOrderSuccess={(order) => {
          setCartOpen(false);
          onOrderConfirmed(order);
          onClearCart();
        }}
      />

      {/* Confirmation Modal */}
      {confirmedOrder && (
        <OrderConfirmModal
          order={confirmedOrder}
          counters={counters}
          onClose={onClearConfirmed}
          onTrack={goTrack}
        />
      )}

      {/* Floating cart pill */}
      {cartCount > 0 && !isCartOpen && (
        <button
          onClick={() => setCartOpen(true)}
          className="fixed bottom-6 left-1/2 -translate-x-1/2 z-40 flex items-center gap-3 px-5 py-3 font-semibold text-sm text-white cursor-pointer"
          style={{ background: 'var(--primary)', borderRadius: '999px', boxShadow: '0 4px 20px rgba(240,120,32,0.35)' }}
        >
          <ShoppingCart className="w-4 h-4" />
          <span
            className="inline-flex items-center justify-center w-6 h-6 text-xs font-bold"
            style={{ background: '#fff', color: 'var(--primary)', borderRadius: '50%' }}
          >
            {cartCount}
          </span>
          Review cart · <span className="num">₹{cartTotal.toFixed(0)}</span>
        </button>
      )}
    </div>
  );
};
