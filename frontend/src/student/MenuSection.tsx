import React, { useState, useMemo } from 'react';
import type { MenuItem, CartItem, Counter } from '../types';
import { Search, Loader2, AlertCircle, Plus, Minus, Utensils } from 'lucide-react';

const CATEGORIES = ['All', 'Breakfast', 'Meals', 'Snacks', 'South Indian', 'Beverages'];

const categoryMap: Record<string, string[]> = {
  All:            [],
  Breakfast:      ['breakfast', 'poha', 'paratha', 'dosa', 'idli', 'medu vada', 'uttapam'],
  Meals:          ['meal', 'rice', 'fried rice', 'kolam', 'noodle', 'noodles', 'paratha', 'dosa', 'uttapam'],
  Snacks:         ['snack', 'poha', 'patties', 'samosa', 'vada pav', 'sabudana', 'sandwich', 'fruit'],
  'South Indian': ['south indian', 'dosa', 'idli', 'medu vada', 'uttapam', 'sambhar'],
  Beverages:      ['beverage', 'shake', 'coffee', 'tea', 'drink'],
};

const ITEM_IMAGE_MAP: Record<string, string> = {
  'poha':               '/images/menu/poha.jpg',
  'fruit plate':        '/images/menu/fruit-plate.jpg',
  'patties':            '/images/menu/patties.jpg',
  'uttapam':            '/images/menu/uttapam.jpg',
  'samosa':             '/images/menu/samosa.jpg',
  'samosa (2 pcs)':     '/images/menu/samosa.jpg',
  'medu vada':          '/images/menu/medu-vada.jpg',
  'vada pav':           '/images/menu/vada-pav.jpg',
  'idli sambhar':       '/images/menu/idli-sambhar.jpg',
  'sabudana vada':      '/images/menu/sabudana-vada.jpg',
  'sabudana khichdi':   '/images/menu/sabudana-khichdi.jpg',
  'aloo paratha':       '/images/menu/aloo-paratha.jpg',
  'paneer paratha':     '/images/menu/paneer-paratha.jpg',
  'plain dosa':         '/images/menu/plain-dosa.jpg',
  'masala dosa':        '/images/menu/masala-dosa.jpg',
  'plain sandwich':     '/images/menu/plain-sandwich.jpg',
  'paneer sandwich':    '/images/menu/paneer-sandwich.jpg',
  'fried rice':         '/images/menu/fried-rice.jpg',
  'veg fried rice':     '/images/menu/fried-rice.jpg',
  'kolam rice':         '/images/menu/kolam-rice.jpg',
  'noodles':            '/images/menu/noodles.jpg',
  'spicy noodles':      '/images/menu/spicy-noodles.jpg',
  'chikoo shake':       '/images/menu/chikoo-shake.jpg',
  'mango shake':        '/images/menu/mango-shake.jpg',
  'banana shake':       '/images/menu/banana-shake.jpg',
  'pineapple shake':    '/images/menu/pineapple-shake.jpg',
  'anjeer shake':       '/images/menu/anjeer-shake.jpg',
  'strawberry shake':   '/images/menu/strawberry-shake.jpg',
  'cold coffee':        '/images/menu/cold-coffee.jpg',
  'thick cold coffee':  '/images/menu/thick-cold-coffee.jpg',
  'butterscotch shake': '/images/menu/butterscotch-shake.jpg',
};

export function getItemImage(name: string): string | null {
  const n = name.toLowerCase().trim();
  if (ITEM_IMAGE_MAP[n]) return ITEM_IMAGE_MAP[n];
  for (const [key, path] of Object.entries(ITEM_IMAGE_MAP)) {
    if (n.includes(key) || key.includes(n)) return path;
  }
  return null;
}

function matchCategory(item: MenuItem, cat: string): boolean {
  if (cat === 'All') return true;
  const keywords = categoryMap[cat] || [];
  const text = (item.name + ' ' + item.category).toLowerCase();
  return keywords.some(k => text.includes(k));
}

const VCAT_WORDS = ['veg', 'paneer', 'dal', 'salad', 'fruit', 'juice', 'lassi', 'water', 'tea', 'coffee', 'dosa', 'idli'];
function isVeg(item: MenuItem): boolean {
  const t = (item.name + ' ' + item.category).toLowerCase();
  return VCAT_WORDS.some(w => t.includes(w));
}

interface Props {
  menu: MenuItem[];
  loading: boolean;
  error: string | null;
  onRetry: () => void;
  cart: CartItem[];
  onAdd: (item: MenuItem) => void;
  onUpdateQty: (id: number, delta: number) => void;
  counters: Counter[];
  compact: boolean;
  onOpenCart: () => void;
  onViewAll: () => void;
}

export const MenuSection: React.FC<Props> = ({
  menu, loading, error, onRetry, cart, onAdd, onUpdateQty,
  compact, onViewAll,
}) => {
  const [search, setSearch]       = useState('');
  const [activecat, setActivecat] = useState('All');

  const visible = useMemo(() => {
    const q = search.toLowerCase();
    return menu
      .filter(i => i.is_available)
      .filter(i => !q || i.name.toLowerCase().includes(q) || i.category.toLowerCase().includes(q))
      .filter(i => matchCategory(i, activecat));
  }, [menu, search, activecat]);

  const cartMap = useMemo(
    () => Object.fromEntries(cart.map(ci => [ci.item.id, ci.quantity])),
    [cart]
  );

  if (loading) {
    return (
      <div className="py-24 flex flex-col items-center gap-3">
        <Loader2 className="w-8 h-8 animate-spin" style={{ color: 'var(--primary)' }} />
        <p className="text-sm" style={{ color: 'var(--muted)' }}>Loading menu…</p>
      </div>
    );
  }

  if (error) {
    return (
      <div className="py-16 flex flex-col items-center gap-3 px-4">
        <AlertCircle className="w-10 h-10" style={{ color: 'var(--pill-negative-ink)' }} />
        <p className="font-medium text-center" style={{ color: 'var(--text)' }}>Cannot reach canteen server</p>
        <p className="text-sm text-center" style={{ color: 'var(--muted)' }}>{error}</p>
        <button
          onClick={onRetry}
          className="btn-orange mt-1 px-6 py-2.5 text-sm"
        >
          Try again
        </button>
      </div>
    );
  }

  return (
    <section className="max-w-6xl mx-auto px-4 sm:px-6 pb-20">
      {/* Section heading row */}
      <div className="pt-10 pb-6 flex flex-col sm:flex-row sm:items-center justify-between gap-4">
        <h2 className="font-display text-3xl" style={{ color: 'var(--text)' }}>
          Our Menu
        </h2>
        <div className="relative sm:w-64">
          <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-3.5 h-3.5" style={{ color: 'var(--faint)' }} />
          <input
            type="text"
            placeholder="Search dishes…"
            value={search}
            onChange={e => setSearch(e.target.value)}
            className="w-full pl-9 pr-3 text-sm border outline-none transition"
            style={{
              background: 'var(--surface)',
              borderColor: 'var(--border)',
              color: 'var(--text)',
              borderRadius: '10px',
              height: '38px',
            }}
          />
        </div>
      </div>

      {/* Category filter chips */}
      <div className="flex gap-2 overflow-x-auto pb-6 scrollbar-none">
        {CATEGORIES.map(cat => (
          <button
            key={cat}
            onClick={() => setActivecat(cat)}
            className="shrink-0 px-4 py-2 text-sm font-medium transition whitespace-nowrap cursor-pointer border"
            style={{
              background:   activecat === cat ? 'var(--primary)' : 'var(--raised)',
              color:        activecat === cat ? '#fff'            : 'var(--muted)',
              borderColor:  activecat === cat ? 'var(--primary)'  : 'var(--border)',
              borderRadius: '999px',
            }}
          >
            {cat}
          </button>
        ))}
      </div>

      {/* Grid */}
      {visible.length === 0 ? (
        <div
          className="py-10 text-center border"
          style={{ borderColor: 'var(--border)', borderStyle: 'dashed', borderRadius: '16px' }}
        >
          <p className="text-sm" style={{ color: 'var(--muted)' }}>Nothing found — try a different search or category</p>
        </div>
      ) : (
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-6" style={{ paddingTop: '56px' }}>
          {(compact ? visible.slice(0, 6) : visible).map((item, idx) => {
            const qty = cartMap[item.id] ?? 0;
            return (
              <FoodCard
                key={item.id}
                item={item}
                qty={qty}
                onAdd={onAdd}
                onUpdateQty={onUpdateQty}
                animIndex={idx}
              />
            );
          })}
        </div>
      )}

      {/* View all / See more */}
      {compact && visible.length > 6 && (
        <div className="mt-10 text-center">
          <button
            type="button"
            onClick={onViewAll}
            className="px-7 py-3 text-sm font-semibold border-2 cursor-pointer transition"
            style={{
              borderColor: 'var(--primary)',
              color: 'var(--primary)',
              background: 'transparent',
              borderRadius: '10px',
            }}
          >
            View full menu — {visible.length} items
          </button>
        </div>
      )}
    </section>
  );
};

// ── Food Card ──────────────────────────────────────────────────────────────────
const FoodCard: React.FC<{
  item: MenuItem;
  qty: number;
  onAdd: (item: MenuItem) => void;
  onUpdateQty: (id: number, delta: number) => void;
  animIndex?: number;
}> = ({ item, qty, onAdd, onUpdateQty, animIndex = 0 }) => {
  const veg     = isVeg(item);
  const itemImg = getItemImage(item.name);
  const [imgError, setImgError] = useState(false);

  return (
    <div
      className="relative flex flex-col card-hover animate-card-enter"
      style={{
        background: 'var(--surface)',
        borderRadius: '24px',
        boxShadow: '0 4px 24px rgba(240,120,32,0.08)',
        paddingTop: '60px',
        border: '1px solid var(--border)',
        animationDelay: `${animIndex * 60}ms`,
      }}
    >
      {/* Circular image — floats above card */}
      <div
        style={{
          position: 'absolute',
          top: -50,
          left: '50%',
          transform: 'translateX(-50%)',
        }}
      >
        {itemImg && !imgError ? (
          <img
            src={itemImg}
            alt={item.name}
            className="food-img-circle"
            loading="lazy"
            onError={() => setImgError(true)}
            style={{ transition: 'transform 0.3s ease' }}
          />
        ) : (
          <div
            className="food-img-circle flex items-center justify-center"
            style={{ background: 'var(--raised)' }}
          >
            <Utensils className="w-10 h-10" style={{ color: 'var(--faint)' }} />
          </div>
        )}
      </div>

      {/* Veg pill */}
      <div className="flex justify-center">
        <span
          className="text-[11px] font-medium px-2.5 py-0.5"
          style={veg
            ? { background: 'var(--pill-positive-bg)', color: 'var(--pill-positive-ink)', borderRadius: '6px' }
            : { background: 'var(--pill-warning-bg)',  color: 'var(--pill-warning-ink)',  borderRadius: '6px' }
          }
        >
          {veg ? 'Veg' : 'Non-veg'}
        </span>
      </div>

      {/* Body */}
      <div className="px-4 pt-2 pb-1 flex flex-col gap-1 flex-1 text-center">
        <h3 className="font-semibold text-base leading-tight" style={{ color: 'var(--text)' }}>{item.name}</h3>
        <p className="text-xs capitalize" style={{ color: 'var(--faint)' }}>{item.category}</p>
        <p className="text-xs" style={{ color: 'var(--muted)' }}>~{item.prep_time_minutes} min prep</p>
      </div>

      {/* Footer */}
      <div className="px-4 pb-4 flex items-center justify-between mt-auto">
        <span className="font-bold text-base num" style={{ color: 'var(--text)' }}>₹{item.price}</span>

        {qty === 0 ? (
          <button
            onClick={() => onAdd(item)}
            className="flex items-center gap-1.5 px-4 py-2 text-sm font-semibold text-white cursor-pointer transition"
            style={{ background: 'var(--primary)', borderRadius: '10px', border: 'none' }}
          >
            <Plus className="w-3.5 h-3.5" />
            Add to Cart
          </button>
        ) : (
          <div
            className="flex items-center border overflow-hidden"
            style={{ borderColor: 'var(--primary)', borderRadius: '10px' }}
          >
            <button
              onClick={() => onUpdateQty(item.id, -1)}
              className="w-8 h-8 flex items-center justify-center cursor-pointer transition"
              style={{ color: 'var(--primary)', background: 'var(--raised)', border: 'none' }}
            >
              <Minus className="w-3.5 h-3.5" />
            </button>
            <span className="w-7 text-center text-sm font-semibold num" style={{ color: 'var(--text)' }}>{qty}</span>
            <button
              onClick={() => onUpdateQty(item.id, +1)}
              className="w-8 h-8 flex items-center justify-center cursor-pointer transition"
              style={{ color: '#fff', background: 'var(--primary)', border: 'none' }}
            >
              <Plus className="w-3.5 h-3.5" />
            </button>
          </div>
        )}
      </div>
    </div>
  );
};
