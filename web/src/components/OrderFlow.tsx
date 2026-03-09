import { useStore } from '../store/useStore';
import { ClipboardList } from 'lucide-react';

export function OrderFlow() {
    const { metrics } = useStore();

    return (
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5 shadow-lg">
            <div className="flex items-center justify-between mb-4">
                <h2 className="text-sm font-semibold text-slate-400 uppercase tracking-wider flex items-center gap-2">
                    <ClipboardList className="w-4 h-4" /> Order Flow
                </h2>
            </div>

            <div className="grid grid-cols-2 gap-3">
                <StatBox label="Submitted" value={metrics.orders_submitted} color="text-blue-400" />
                <StatBox label="Accepted" value={metrics.orders_approved} color="text-green-400" />
                <StatBox label="Filled" value={metrics.orders_filled} color="text-emerald-400" />
                <StatBox label="Rejected" value={metrics.orders_rejected} color="text-red-400" />
                <StatBox label="Denied" value={metrics.orders_denied} color="text-orange-400" />
                <StatBox label="Pending" value={metrics.orders_submitted - metrics.orders_filled - metrics.orders_rejected} color="text-yellow-400" />
            </div>
        </div>
    );
}

function StatBox({ label, value, color }: any) {
    return (
        <div className="bg-slate-950 p-3 rounded-lg border border-slate-800">
            <div className="text-xs text-slate-500 mb-1">{label}</div>
            <div className={`text-xl font-bold font-mono ${color}`}>{value.toLocaleString()}</div>
        </div>
    );
}
