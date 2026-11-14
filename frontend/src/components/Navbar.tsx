import React from 'react';
import {
  UtensilsCrossed,
  LayoutDashboard,
  Search,
  ShieldAlert,
  ShoppingCart,
  Wifi,
  WifiOff,
  MapPin,
  ChevronDown,
} from 'lucide-react';
import type { HealthStatus } from '../types';

interface Props {
  activeTab: string;
  setActiveTab: (tab: string) => void;
  cartCount: number;
  openCart: () => void;
  health: HealthStatus | null;
  healthLoading: boolean;
  isStaffAuthenticated?: boolean;
}

export const Navbar: React.FC<Props> = ({
  activeTab,
  setActiveTab,
  cartCount,
  openCart,
  health,
  healthLoading,
  isStaffAuthenticated = false,
}) => {
  const isOnline = health !== null && health.status === 'healthy';

  const navItems = [
    { id: 'menu', label: 'Menu & Order', icon: UtensilsCrossed },
    { id: 'queues', label: 'Live Queues', icon: LayoutDashboard },
    { id: 'track', label: 'Track Order', icon: Search },
    { id: 'admin', label: 'Staff & Admin Ops', icon: ShieldAlert },
  ];

  return (
    <header className="sticky top-0 z-40 bg-[#FFFDF8]/95 backdrop-blur-md border-b border-[#E9DED1] shadow-xs">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-18">
          {/* Left: Brand & Location Context */}
          <div className="flex items-center gap-4 sm:gap-6">
            <div
              className="flex items-center gap-2.5 cursor-pointer select-none group"
              onClick={() => setActiveTab('menu')}
            >
              <div className="w-10 h-10 rounded-xl bg-[#C95725] text-white flex items-center justify-center shadow-xs group-hover:bg-[#B3471A] transition-colors">
                <UtensilsCrossed className="w-5 h-5 text-white" />
              </div>
              <div>
                <div className="flex items-center gap-1.5">
                  <span className="font-serif font-black text-xl tracking-tight text-[#33251D]">
                    Campus<span className="text-[#C95725]">Dine</span>
                  </span>
                  <span className="hidden xl:inline-flex items-center text-[10px] font-bold uppercase tracking-wider px-2 py-0.5 rounded bg-[#FDF0E9] text-[#C95725] border border-[#F4D1BD]">
                    College Canteen
                  </span>
                </div>
                <p className="text-[11px] font-medium text-[#81746A] hidden sm:block">
                  AI-Optimized Queue & Kitchen Management
                </p>
              </div>
            </div>

            {/* Campus Canteen Location Context */}
            <div className="hidden md:flex items-center gap-2 pl-4 border-l border-[#E9DED1] text-xs">
              <MapPin className="w-4 h-4 text-[#C95725] shrink-0" />
              <div className="flex flex-col">
                <span className="font-bold text-[#33251D] flex items-center gap-1">
                  Campus Central Canteen
                  <ChevronDown className="w-3 h-3 text-[#81746A]" />
                </span>
                <span className="text-[11px] text-[#81746A] truncate max-w-[170px]">
                  Ground Floor, Student Activity Hub
                </span>
              </div>
            </div>
          </div>

          {/* Navigation Tabs */}
          <nav className="flex items-center gap-1 sm:gap-1.5">
            {navItems.map((item) => {
              const Icon = item.icon;
              const isActive = activeTab === item.id;
              const isStaffTab = item.id === 'admin';

              return (
                <button
                  key={item.id}
                  onClick={() => setActiveTab(item.id)}
                  className={`flex items-center gap-2 px-3.5 py-2 rounded-xl text-xs sm:text-sm font-bold transition-all cursor-pointer ${
                    isActive
                      ? 'bg-[#C95725] text-[#FFFDF8] shadow-xs'
                      : 'text-[#81746A] hover:text-[#33251D] hover:bg-[#F8F1E8]'
                  }`}
                >
                  <Icon className={`w-4 h-4 ${isActive ? 'text-[#FFFDF8]' : 'text-[#81746A]'}`} />
                  <span className="hidden md:inline">{item.label}</span>
                  {isStaffTab && isStaffAuthenticated && (
                    <span className="w-2 h-2 rounded-full bg-emerald-400" title="Staff Logged In" />
                  )}
                </button>
              );
            })}
          </nav>

          {/* Right Controls: Backend Status & Cart */}
          <div className="flex items-center gap-3">
            {/* Backend Connection Badge */}
            <div
              className={`hidden lg:flex items-center gap-1.5 px-3 py-1.5 rounded-full text-xs font-bold border ${
                healthLoading
                  ? 'bg-[#F8F1E8] text-[#81746A] border-[#E9DED1]'
                  : isOnline
                  ? 'bg-[#EFF8F1] text-[#2D663E] border-[#CFEAD5]'
                  : 'bg-[#FDF0F0] text-[#9E3939] border-[#F4CECE]'
              }`}
              title={
                isOnline
                  ? `Backend: C++ REST API\nML: ${health?.ml_model_source}\nDB: SQLite Connected`
                  : 'Backend unreachable. Run canteen_app.exe --server 8080'
              }
            >
              {healthLoading ? (
                <span className="w-2 h-2 rounded-full bg-[#81746A] animate-pulse" />
              ) : isOnline ? (
                <>
                  <span className="w-2 h-2 rounded-full bg-emerald-500 animate-pulse" />
                  <Wifi className="w-3.5 h-3.5 text-[#2D663E]" />
                  <span>C++ Engine Online</span>
                </>
              ) : (
                <>
                  <WifiOff className="w-3.5 h-3.5 text-[#9E3939]" />
                  <span>Engine Offline</span>
                </>
              )}
            </div>

            {/* Cart Button */}
            <button
              onClick={openCart}
              className="relative flex items-center gap-2 px-3.5 py-2 rounded-xl bg-[#33251D] hover:bg-[#251A14] text-[#FFFDF8] font-bold text-xs sm:text-sm transition-all shadow-xs cursor-pointer"
              aria-label="View Cart"
            >
              <ShoppingCart className="w-4 h-4 text-[#FFFDF8]" />
              <span className="hidden sm:inline">Cart</span>
              {cartCount > 0 && (
                <span className="bg-[#C95725] text-white font-black text-xs px-2 py-0.5 rounded-full min-w-[20px] text-center shadow-xs">
                  {cartCount}
                </span>
              )}
            </button>
          </div>
        </div>
      </div>
    </header>
  );
};
