import React from 'react';

export const StudentFooter: React.FC = () => (
  <footer className="mt-16 py-8 text-xs" style={{ background: '#1C1006' }}>
    <div className="max-w-6xl mx-auto px-4 sm:px-6 flex flex-col sm:flex-row items-center justify-between gap-4">
      <div className="flex flex-col items-center sm:items-start gap-1">
        <span className="font-display text-lg" style={{ color: '#FEF7ED' }}>
          Canteen<span style={{ color: 'var(--primary)' }}>Dine</span>
        </span>
        <span style={{ color: '#C4A47A' }}>AI-Based College Canteen Queue Optimization</span>
      </div>
      <div className="flex flex-wrap gap-2 items-center justify-center">
        {['C++ DSA', 'ML Prediction', 'SQLite', '© 2026'].map(t => (
          <span key={t} className="px-2.5 py-1 border" style={{ borderColor: '#3D2E1A', color: '#C4A47A', borderRadius: '6px' }}>{t}</span>
        ))}
      </div>
    </div>
  </footer>
);
