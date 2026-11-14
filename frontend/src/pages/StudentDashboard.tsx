import React, { useState } from 'react';
import {
  Search,
  Clock,
  Sparkles,
  Plus,
  Minus,
  ChefHat,
  RefreshCw,
  AlertTriangle,
  ArrowRight,
  TrendingUp,
  Star,
  ShieldCheck,
} from 'lucide-react';
import type { MenuItem, CartItem, Counter } from '../types';

interface Props {
  menu: MenuItem[];
  loading: boolean;
  error: string | null;
  onRetry: () => void;
  cart: CartItem[];
  onAddToCart: (item: MenuItem) => void;
  onUpdateQuantity: (itemId: number, delta: number) => void;
  onOpenCart: () => void;
  counters: Counter[];
}

// Curated high-resolution food photography for college canteen items
const FOOD_IMAGES: Record<string, string> = {
  'Veg Cheese Burger': 'https://images.unsplash.com/photo-1568901346375-23c9450c58cd?auto=format&fit=crop&w=600&q=80',
  'Paneer Kathi Roll': 'https://images.unsplash.com/photo-1626777552726-4a6b54c97e46?auto=format&fit=crop&w=600&q=80',
  'Masala Dosa': 'https://images.unsplash.com/photo-1668236543090-82eba5ee5976?auto=format&fit=crop&w=600&q=80',
  'Veg Fried Rice': 'https://images.unsplash.com/photo-1603133872878-684f208fb84b?auto=format&fit=crop&w=600&q=80',
  'Cold Coffee': 'https://images.unsplash.com/photo-1517701550927-30cf4ba1dba5?auto=format&fit=crop&w=600&q=80',
  'Masala Chai': 'https://images.unsplash.com/photo-1576092768241-dec231879fc3?auto=format&fit=crop&w=600&q=80',
  'Samosa Plate': 'https://images.unsplash.com/photo-1601050690597-df0568f70950?auto=format&fit=crop&w=600&q=80',
  'Chocolate Pastry': 'https://images.unsplash.com/photo-1578985545062-69928b1d9587?auto=format&fit=crop&w=600&q=80',
};

// Fallback food banner if image fails to load
const DEFAULT_FOOD_IMAGE = 'https://images.unsplash.com/photo-1546069901-ba9599a7e63c?auto=format&fit=crop&w=600&q=80';

// Category icons for quick visual recognition
const CATEGORY_ICONS: Record<string, string> = {
  All: '🍽️',
  'Fast Food': '🍔',
  'South Indian': '🥞',
  'Meals & Chinese': '🍚',
  Beverages: '☕',
  'Bakery & Snacks': '🥐',
};

export const StudentDashboard: React.FC<Props> = ({
  menu,
  loading,
  error,
  onRetry,
  cart,
  onAddToCart,
  onUpdateQuantity,
  onOpenCart,
  counters,
}) => {
  const [selectedCategory, setSelectedCategory] = useState<string>('All');
  const [searchQuery, setSearchQuery] = useState<string>('');

  // Extract unique categories preserving order
  const categories = ['All', ...Array.from(new Set(menu.map((m) => m.category)))];

  // Filter items
  const filteredItems = menu.filter((item) => {
    const matchesCategory =
      selectedCategory === 'All' || item.category === selectedCategory;
    const matchesSearch =
      item.name.toLowerCase().includes(searchQuery.toLowerCase()) ||
      item.category.toLowerCase().includes(searchQuery.toLowerCase());
    return matchesCategory && matchesSearch;
  });

  // Real "Popular on Campus" items from the menu
  const popularItemNames = ['Masala Dosa', 'Veg Cheese Burger', 'Cold Coffee', 'Samosa Plate'];
  const popularItems = menu.filter((m) => popularItemNames.includes(m.name) && m.is_available);

  const cartTotal = cart.reduce((sum, ci) => sum + ci.item.price * ci.quantity, 0);
  const cartItemCount = cart.reduce((sum, ci) => sum + ci.quantity, 0);

  return (
    <div className="space-y-8 animate-fadeIn pb-16">
      {/* Warm Editorial Welcome Section (Balanced, not oversized) */}
      <section className="bg-[#FFFDF8] border border-[#E9DED1] rounded-3xl p-6 sm:p-8 shadow-xs">
        <div className="flex flex-col md:flex-row md:items-center justify-between gap-6">
          <div className="space-y-2 max-w-xl">
            <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-[#FDF0E9] text-[#C95725] border border-[#F4D1BD] text-xs font-bold tracking-wide">
              <Sparkles className="w-3.5 h-3.5" />
              <span>AI Queue Optimization Active • Zero Physical Line</span>
            </div>

            <h1 className="font-serif text-3xl sm:text-4xl font-extrabold text-[#33251D] tracking-tight leading-tight">
              What are you craving today?
            </h1>

            <p className="text-sm text-[#81746A] leading-relaxed">
              Order fresh college canteen meals directly to your phone. Our C++ scheduling engine predicts prep times and distributes orders across {counters.length || 4} counters to minimize waiting.
            </p>
          </div>

          {/* Real-time Counter Workload Indicators */}
          <div className="bg-[#F8F1E8] p-4 rounded-2xl border border-[#E9DED1] space-y-2 shrink-0 md:min-w-[240px]">
            <div className="flex items-center justify-between text-xs font-bold text-[#33251D]">
              <span>Live Counters</span>
              <span className="text-[11px] text-[#81746A] font-normal">FIFO Queues</span>
            </div>
            <div className="grid grid-cols-2 gap-2 text-xs">
              {counters.map((c) => (
                <div
                  key={c.id}
                  className="flex items-center gap-1.5 px-2.5 py-1.5 rounded-xl bg-[#FFFDF8] border border-[#E9DED1] text-xs font-semibold text-[#33251D]"
                >
                  <span
                    className={`w-2 h-2 rounded-full ${
                      c.is_available ? 'bg-emerald-500' : 'bg-red-400'
                    }`}
                  />
                  <span>C{c.id}</span>
                  <span className="text-[#81746A] font-mono text-[11px]">({c.pending_orders_count}q)</span>
                </div>
              ))}
            </div>
          </div>
        </div>
      </section>

      {/* Search & Category Filter Section */}
      <section className="space-y-4">
        <div className="flex flex-col md:flex-row gap-3 items-center justify-between">
          {/* Prominent Search Bar */}
          <div className="relative w-full md:max-w-md">
            <Search className="w-4 h-4 absolute left-3.5 top-1/2 -translate-y-1/2 text-[#81746A]" />
            <input
              type="text"
              placeholder="Search dishes (e.g. Burger, Dosa, Coffee...)"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="w-full pl-10 pr-4 py-2.5 rounded-xl bg-[#FFFDF8] border border-[#E9DED1] text-sm text-[#33251D] placeholder-[#81746A] focus:outline-none focus:border-[#C95725] focus:ring-1 focus:ring-[#C95725] transition shadow-xs"
            />
          </div>

          {/* Quick Dietary Pill */}
          <div className="hidden sm:flex items-center gap-2 text-xs font-bold text-[#33251D] bg-[#FFFDF8] px-3.5 py-2 rounded-xl border border-[#E9DED1] shadow-xs">
            <ShieldCheck className="w-4 h-4 text-emerald-600" />
            <span>100% Pure Vegetarian Campus Kitchen</span>
          </div>
        </div>

        {/* Horizontally Scrollable Category Chips */}
        <div>
          <div className="text-xs font-bold uppercase tracking-wider text-[#81746A] mb-2.5">
            Canteen Categories
          </div>
          <div className="flex items-center gap-2.5 overflow-x-auto pb-1 scrollbar-none">
            {categories.map((cat) => {
              const isSelected = selectedCategory === cat;
              const icon = CATEGORY_ICONS[cat] || '🍲';
              return (
                <button
                  key={cat}
                  onClick={() => setSelectedCategory(cat)}
                  className={`group flex items-center gap-2 px-3.5 py-2 rounded-xl text-xs sm:text-sm font-bold transition whitespace-nowrap cursor-pointer ${
                    isSelected
                      ? 'bg-[#C95725] text-[#FFFDF8] shadow-xs scale-[1.02]'
                      : 'bg-[#FFFDF8] text-[#33251D] hover:bg-[#F8F1E8] border border-[#E9DED1]'
                  }`}
                >
                  <span className="text-base group-hover:scale-110 transition-transform">
                    {icon}
                  </span>
                  <span>{cat}</span>
                </button>
              );
            })}
          </div>
        </div>
      </section>

      {/* "Popular on Campus" Row */}
      {!searchQuery && selectedCategory === 'All' && popularItems.length > 0 && (
        <section className="space-y-3">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2">
              <TrendingUp className="w-5 h-5 text-[#C95725]" />
              <h2 className="font-serif text-xl font-bold text-[#33251D] tracking-tight">
                Popular on campus
              </h2>
            </div>
            <span className="text-xs font-medium text-[#81746A]">
              Student favorites & fast-moving orders
            </span>
          </div>

          <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
            {popularItems.map((item) => {
              const inCart = cart.find((ci) => ci.item.id === item.id);
              const imageUrl = FOOD_IMAGES[item.name] || DEFAULT_FOOD_IMAGE;

              return (
                <div
                  key={`pop-${item.id}`}
                  className="group rounded-2xl bg-[#FFFDF8] border border-[#E9DED1] p-3 flex flex-col justify-between shadow-xs hover:shadow-md transition-all duration-200"
                >
                  <div>
                    {/* Food Photo Container */}
                    <div className="relative w-full h-32 rounded-xl overflow-hidden mb-2.5 bg-[#F8F1E8]">
                      <img
                        src={imageUrl}
                        alt={item.name}
                        className="w-full h-full object-cover group-hover:scale-105 transition-transform duration-300"
                        loading="lazy"
                        onError={(e) => {
                          (e.target as HTMLImageElement).src = DEFAULT_FOOD_IMAGE;
                        }}
                      />
                      {/* Veg Indicator Badge */}
                      <div className="absolute top-2 left-2 bg-[#FFFDF8]/95 p-1 rounded-md shadow-xs">
                        <div className="w-3.5 h-3.5 border border-emerald-600 flex items-center justify-center rounded-[3px]">
                          <div className="w-1.5 h-1.5 rounded-full bg-emerald-600" />
                        </div>
                      </div>

                      {/* Prep Time Estimate */}
                      <div className="absolute bottom-2 right-2 bg-[#33251D]/80 backdrop-blur-xs text-[#FFFDF8] text-[10px] font-bold px-2 py-0.5 rounded-md flex items-center gap-1">
                        <Clock className="w-3 h-3 text-[#E9A06D]" />
                        <span>~{item.prep_time_minutes}m</span>
                      </div>
                    </div>

                    <div className="flex items-center justify-between text-xs text-[#81746A] mb-1">
                      <span className="font-bold text-[#C95725] text-[10px] uppercase tracking-wide">
                        {item.category}
                      </span>
                      <span className="flex items-center gap-0.5 font-bold text-[#33251D] text-[11px]">
                        <Star className="w-3 h-3 text-amber-500 fill-amber-500" />
                        <span>4.{item.complexity + 5}</span>
                      </span>
                    </div>

                    <h3 className="font-bold text-[#33251D] group-hover:text-[#C95725] transition-colors text-sm line-clamp-1">
                      {item.name}
                    </h3>
                  </div>

                  <div className="mt-2.5 pt-2.5 border-t border-[#E9DED1]/70 flex items-center justify-between">
                    <div>
                      <span
                        className="text-amber-400 font-extrabold text-base"
                        style={{ color: '#33251D' }}
                      >
                        ₹{item.price.toFixed(2)}
                      </span>
                    </div>

                    {inCart ? (
                      <div className="flex items-center gap-2 bg-[#C95725] text-white px-2 py-1 rounded-xl shadow-xs">
                        <button
                          onClick={() => onUpdateQuantity(item.id, -1)}
                          className="w-5 h-5 flex items-center justify-center hover:bg-[#B3471A] rounded-lg transition"
                          aria-label="Decrease quantity"
                        >
                          <Minus className="w-3 h-3" />
                        </button>
                        <span className="text-xs font-bold min-w-[1.2rem] text-center">
                          {inCart.quantity}
                        </span>
                        <button
                          onClick={() => onUpdateQuantity(item.id, 1)}
                          className="w-5 h-5 flex items-center justify-center hover:bg-[#B3471A] rounded-lg transition"
                          aria-label="Increase quantity"
                        >
                          <Plus className="w-3 h-3" />
                        </button>
                      </div>
                    ) : (
                      <button
                        onClick={() => onAddToCart(item)}
                        className="px-3.5 py-1.5 rounded-xl border border-[#C95725] bg-[#FDF0E9] hover:bg-[#C95725] text-[#C95725] hover:text-[#FFFDF8] font-bold text-xs transition shadow-xs flex items-center gap-1 cursor-pointer"
                      >
                        <Plus className="w-3 h-3" />
                        <span>Add</span>
                      </button>
                    )}
                  </div>
                </div>
              );
            })}
          </div>
        </section>
      )}

      {/* "Explore the menu" Main Section */}
      <section className="space-y-4">
        <div className="flex items-center justify-between">
          <div>
            <h2 className="font-serif text-2xl font-bold text-[#33251D] tracking-tight">
              Explore the menu
            </h2>
            <p className="text-xs text-[#81746A] mt-0.5">
              Showing {filteredItems.length} freshly prepared canteen dishes
            </p>
          </div>
        </div>

        {/* Loading Skeleton */}
        {loading && (
          <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
            {[1, 2, 3, 4, 5, 6, 7, 8].map((n) => (
              <div
                key={n}
                className="rounded-2xl bg-[#FFFDF8] border border-[#E9DED1] p-4 animate-pulse space-y-3"
              >
                <div className="h-36 bg-[#F8F1E8] rounded-xl" />
                <div className="h-4 bg-[#F8F1E8] rounded w-1/3" />
                <div className="h-5 bg-[#F8F1E8] rounded w-3/4" />
                <div className="h-8 bg-[#F8F1E8] rounded-xl mt-4" />
              </div>
            ))}
          </div>
        )}

        {/* Error State */}
        {error && !loading && (
          <div className="p-8 rounded-3xl bg-[#FDF0E9] border border-[#F4D1BD] text-center space-y-3 max-w-lg mx-auto">
            <AlertTriangle className="w-8 h-8 text-[#C95725] mx-auto" />
            <h3 className="text-base font-bold text-[#33251D]">Menu Loading Error</h3>
            <p className="text-xs text-[#81746A]">{error}</p>
            <button
              onClick={onRetry}
              className="inline-flex items-center gap-2 px-4 py-2 rounded-xl bg-[#C95725] hover:bg-[#B3471A] text-white text-xs font-bold transition shadow-xs cursor-pointer"
            >
              <RefreshCw className="w-3.5 h-3.5" />
              <span>Retry Connection</span>
            </button>
          </div>
        )}

        {/* Empty Filter State */}
        {!loading && !error && filteredItems.length === 0 && (
          <div className="text-center py-16 space-y-3 bg-[#FFFDF8] rounded-3xl border border-[#E9DED1]">
            <ChefHat className="w-12 h-12 text-[#81746A] mx-auto opacity-40" />
            <p className="text-base font-bold text-[#33251D]">No dishes found</p>
            <p className="text-xs text-[#81746A]">
              Try searching for a different dish name or reset your category filter.
            </p>
          </div>
        )}

        {/* Menu Cards Grid */}
        {!loading && !error && filteredItems.length > 0 && (
          <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4 sm:gap-5">
            {filteredItems.map((item) => {
              const inCart = cart.find((ci) => ci.item.id === item.id);
              const isAvailable = item.is_available;
              const imageUrl = FOOD_IMAGES[item.name] || DEFAULT_FOOD_IMAGE;

              return (
                <div
                  key={item.id}
                  className={`group rounded-2xl bg-[#FFFDF8] border border-[#E9DED1] p-3.5 flex flex-col justify-between shadow-xs hover:shadow-md transition-all duration-200 ${
                    !isAvailable ? 'opacity-60 bg-[#F8F1E8]' : 'hover:-translate-y-0.5'
                  }`}
                >
                  <div>
                    {/* Food Photo Container */}
                    <div className="relative w-full h-40 rounded-xl overflow-hidden mb-3 bg-[#F8F1E8]">
                      <img
                        src={imageUrl}
                        alt={item.name}
                        className="w-full h-full object-cover group-hover:scale-105 transition-transform duration-300"
                        loading="lazy"
                        onError={(e) => {
                          (e.target as HTMLImageElement).src = DEFAULT_FOOD_IMAGE;
                        }}
                      />

                      {/* Pure Veg Indian Indicator */}
                      <div className="absolute top-2 left-2 bg-[#FFFDF8]/95 p-1 rounded-md shadow-xs">
                        <div className="w-3.5 h-3.5 border border-emerald-600 flex items-center justify-center rounded-[3px]">
                          <div className="w-1.5 h-1.5 rounded-full bg-emerald-600" />
                        </div>
                      </div>

                      {/* Prep Time Estimate */}
                      <div className="absolute bottom-2 right-2 bg-[#33251D]/80 backdrop-blur-xs text-[#FFFDF8] text-[10px] font-bold px-2 py-0.5 rounded-md flex items-center gap-1">
                        <Clock className="w-3 h-3 text-[#E9A06D]" />
                        <span>~{item.prep_time_minutes} mins</span>
                      </div>

                      {!isAvailable && (
                        <div className="absolute inset-0 bg-[#33251D]/60 flex items-center justify-center">
                          <span className="bg-[#C95725] text-white font-bold text-xs px-3 py-1 rounded-full uppercase tracking-wider">
                            Out of Stock
                          </span>
                        </div>
                      )}
                    </div>

                    {/* Category Label */}
                    <div className="flex items-center justify-between text-xs mb-1">
                      <span className="font-bold text-[#C95725] text-[10px] uppercase tracking-wide">
                        {item.category}
                      </span>
                      <span className="text-[11px] text-[#81746A]">
                        Complexity: {'★'.repeat(item.complexity)}{'☆'.repeat(Math.max(0, 5 - item.complexity))}
                      </span>
                    </div>

                    {/* Dish Name */}
                    <h3 className="font-bold text-[#33251D] group-hover:text-[#C95725] transition-colors text-base line-clamp-1">
                      {item.name}
                    </h3>

                    <p className="text-xs text-[#81746A] mt-1 line-clamp-2">
                      Freshly prepared campus canteen meal • Smart queue load-balanced preparation.
                    </p>
                  </div>

                  {/* Price & Action Row */}
                  <div className="mt-3 pt-3 border-t border-[#E9DED1]/70 flex items-center justify-between">
                    <div>
                      <span className="text-[10px] text-[#81746A] uppercase font-semibold block">
                        Price
                      </span>
                      <span
                        className="text-amber-400 font-extrabold text-lg"
                        style={{ color: '#33251D' }}
                      >
                        ₹{item.price.toFixed(2)}
                      </span>
                    </div>

                    {isAvailable ? (
                      inCart ? (
                        <div className="flex items-center gap-2 bg-[#C95725] text-white px-2.5 py-1 rounded-xl shadow-xs">
                          <button
                            type="button"
                            onClick={() => onUpdateQuantity(item.id, -1)}
                            className="w-5 h-5 flex items-center justify-center hover:bg-[#B3471A] rounded-lg transition"
                            aria-label="Decrease quantity"
                          >
                            <Minus className="w-3.5 h-3.5" />
                          </button>
                          <span className="text-xs font-black min-w-[1.2rem] text-center">
                            {inCart.quantity}
                          </span>
                          <button
                            type="button"
                            onClick={() => onUpdateQuantity(item.id, 1)}
                            className="w-5 h-5 flex items-center justify-center hover:bg-[#B3471A] rounded-lg transition"
                            aria-label="Increase quantity"
                          >
                            <Plus className="w-3.5 h-3.5 lucide-plus" />
                          </button>
                        </div>
                      ) : (
                        <button
                          type="button"
                          onClick={() => onAddToCart(item)}
                          className="px-4 py-1.5 rounded-xl border border-[#C95725] bg-[#FDF0E9] hover:bg-[#C95725] text-[#C95725] hover:text-[#FFFDF8] font-bold text-xs transition duration-150 shadow-xs flex items-center gap-1.5 cursor-pointer"
                        >
                          <Plus className="w-3.5 h-3.5" />
                          <span>Add</span>
                        </button>
                      )
                    ) : (
                      <button
                        disabled
                        className="py-1.5 px-3 rounded-xl bg-[#F8F1E8] text-[#81746A] text-xs font-semibold cursor-not-allowed"
                      >
                        Unavailable
                      </button>
                    )}
                  </div>
                </div>
              );
            })}
          </div>
        )}
      </section>

      {/* Floating Bottom Cart Bar */}
      {cartItemCount > 0 && (
        <div className="fixed bottom-6 left-1/2 -translate-x-1/2 z-30 w-full max-w-xl px-4 animate-slideUp">
          <div className="rounded-2xl bg-[#33251D] text-[#FFFDF8] p-4 shadow-xl flex items-center justify-between border border-[#E9DED1]/20">
            <div>
              <div className="text-xs font-semibold text-[#E9A06D]">
                {cartItemCount} {cartItemCount === 1 ? 'item' : 'items'} in order
              </div>
              <div className="text-xl font-black text-[#FFFDF8]">
                ₹{cartTotal.toFixed(2)}
              </div>
            </div>
            <button
              onClick={onOpenCart}
              className="py-2.5 px-5 rounded-xl bg-[#C95725] hover:bg-[#B3471A] text-white font-bold text-sm flex items-center gap-2 transition shadow-md cursor-pointer"
            >
              <span>Review Order & Pay</span>
              <ArrowRight className="w-4 h-4" />
            </button>
          </div>
        </div>
      )}
    </div>
  );
};
