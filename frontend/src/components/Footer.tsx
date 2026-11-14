import React from 'react';
import { Cpu, Database, Server, GitFork, Heart } from 'lucide-react';

export const Footer: React.FC = () => {
  return (
    <footer className="mt-auto border-t border-[#E9DED1] bg-[#FFFDF8] py-8 text-xs text-[#81746A]">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex flex-col md:flex-row items-center justify-between gap-4">
          <div>
            <div className="flex items-center gap-2 text-[#33251D] font-bold text-sm">
              <span className="font-serif">Campus<span className="text-[#C95725]">Dine</span></span>
              <span className="text-[#81746A] font-normal">— AI-Based College Canteen Queue Optimization System</span>
            </div>
            <p className="text-[#81746A] mt-1 text-[11px]">
              DSA Course Project • Custom Arrays, Linked Lists, FIFO Queues, Stacks, Searching & Sorting • Random Forest ML • SQLite
            </p>
          </div>

          <div className="flex flex-wrap items-center gap-2.5 text-[#33251D] font-semibold text-[11px]">
            <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-lg bg-[#F8F1E8] border border-[#E9DED1] text-[#33251D]">
              <Server className="w-3.5 h-3.5 text-[#C95725]" /> C++17 REST API
            </span>
            <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-lg bg-[#F8F1E8] border border-[#E9DED1] text-[#33251D]">
              <Cpu className="w-3.5 h-3.5 text-[#E9A06D]" /> Random Forest Predictor
            </span>
            <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-lg bg-[#F8F1E8] border border-[#E9DED1] text-[#33251D]">
              <Database className="w-3.5 h-3.5 text-[#2D663E]" /> SQLite DB
            </span>
            <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-lg bg-[#F8F1E8] border border-[#E9DED1] text-[#33251D]">
              <GitFork className="w-3.5 h-3.5 text-[#33251D]" /> In-Memory DSA Engine
            </span>
          </div>
        </div>

        <div className="mt-6 pt-4 border-t border-[#E9DED1]/70 flex flex-col sm:flex-row items-center justify-between text-[11px] text-[#81746A] gap-2">
          <span>© 2026 CampusDine. Operational Food Service & Queue System.</span>
          <span className="flex items-center gap-1">
            Engineered with <Heart className="w-3 h-3 text-[#C95725] fill-[#C95725]" /> for College Food Service Optimization
          </span>
        </div>
      </div>
    </footer>
  );
};
