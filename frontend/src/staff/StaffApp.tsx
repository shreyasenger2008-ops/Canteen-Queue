import React from 'react';
import { AdminDashboard } from '../pages/AdminDashboard';

interface Props {
  counters?: any[];
  health?: any;
  onGoStudent: () => void;
}

export const StaffApp: React.FC<Props> = ({ onGoStudent }) => {
  return (
    <div className="staff-mode min-h-screen" style={{ background: 'var(--ground)' }}>
      <AdminDashboard onGoStudent={onGoStudent} />
    </div>
  );
};
