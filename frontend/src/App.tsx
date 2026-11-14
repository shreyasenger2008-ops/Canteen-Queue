import React, { useState, useEffect } from 'react';
import type { MenuItem, CartItem, Order, Counter, HealthStatus } from './types';
import { getMenu, getCounters, getHealth } from './services/api';

// ── Student App ──────────────────────────────────────────────────────────────
import { StudentApp } from './student/StudentApp';

// ── Staff App ─────────────────────────────────────────────────────────────────
import { StaffApp } from './staff/StaffApp';

function getInitialRoute(): 'student' | 'staff' {
  if (typeof window !== 'undefined') {
    const p = window.location.pathname.toLowerCase();
    if (p === '/staff' || p.startsWith('/staff')) return 'staff';
    const h = window.location.hash.toLowerCase();
    if (h === '#staff') return 'staff';
  }
  return 'student';
}

export const App: React.FC = () => {
  const [route, setRoute] = useState<'student' | 'staff'>(getInitialRoute);

  // Shared data loaded once
  const [menu, setMenu]         = useState<MenuItem[]>([]);
  const [counters, setCounters] = useState<Counter[]>([]);
  const [health, setHealth]     = useState<HealthStatus | null>(null);
  const [loading, setLoading]   = useState(true);
  const [menuError, setMenuError] = useState<string | null>(null);

  // Cart (owned by App, passed to StudentApp)
  const [cart, setCart]           = useState<CartItem[]>([]);
  const [confirmedOrder, setConfirmedOrder] = useState<Order | null>(null);

  const loadData = async () => {
    setLoading(true);
    setMenuError(null);
    try {
      const [h, m, c] = await Promise.all([
        getHealth().catch(() => null),
        getMenu(),
        getCounters().catch(() => []),
      ]);
      setHealth(h);
      setMenu(m);
      setCounters(c);
    } catch (e: any) {
      setMenuError(e.message || 'Cannot reach backend.');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    loadData();
    const t = setInterval(() => getHealth().then(setHealth).catch(() => setHealth(null)), 20000);
    return () => clearInterval(t);
  }, []);

  useEffect(() => {
    const handle = () => {
      const p = window.location.pathname.toLowerCase();
      setRoute(p.startsWith('/staff') ? 'staff' : 'student');
    };
    window.addEventListener('popstate', handle);
    return () => window.removeEventListener('popstate', handle);
  }, []);

  const navigate = (r: 'student' | 'staff') => {
    setRoute(r);
    window.history.pushState(null, '', r === 'staff' ? '/staff' : '/');
  };

  // Cart helpers
  const addToCart = (item: MenuItem) =>
    setCart(p => {
      const ex = p.find(ci => ci.item.id === item.id);
      if (ex) return p.map(ci => ci.item.id === item.id ? { ...ci, quantity: ci.quantity + 1 } : ci);
      return [...p, { item, quantity: 1 }];
    });

  const updateQty = (id: number, delta: number) =>
    setCart(p =>
      p.map(ci => ci.item.id === id ? { ...ci, quantity: ci.quantity + delta } : ci)
        .filter(ci => ci.quantity > 0)
    );

  const removeItem = (id: number) => setCart(p => p.filter(ci => ci.item.id !== id));
  const clearCart  = ()          => setCart([]);

  if (route === 'staff') {
    return (
      <StaffApp
        counters={counters}
        health={health}
        onGoStudent={() => navigate('student')}
      />
    );
  }

  return (
    <StudentApp
      menu={menu}
      counters={counters}
      health={health}
      loading={loading}
      menuError={menuError}
      onRetry={loadData}
      cart={cart}
      onAddToCart={addToCart}
      onUpdateQty={updateQty}
      onRemoveItem={removeItem}
      onClearCart={clearCart}
      confirmedOrder={confirmedOrder}
      onOrderConfirmed={setConfirmedOrder}
      onClearConfirmed={() => setConfirmedOrder(null)}
      onGoStaff={() => navigate('staff')}
    />
  );
};

export default App;
