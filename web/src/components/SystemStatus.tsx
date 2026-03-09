import { useStore } from '../store/useStore';
import { cn } from '../lib/utils';
import { Cpu, Clock } from 'lucide-react';

export function SystemStatus() {
    const { metrics } = useStore();

    const getStatusColor = (state: number) => {
        switch (state) {
            case 2: return 'text-green-400 bg-green-400/10 border-green-400/20'; // Active/Ready
            case 1: return 'text-yellow-400 bg-yellow-400/10 border-yellow-400/20'; // Initializing
            case 0: return 'text-red-400 bg-red-400/10 border-red-400/20'; // Stopped/Error
            default: return 'text-slate-400 bg-slate-400/10 border-slate-400/20';
        }
    };

    const getStatusText = (state: number) => {
        switch (state) {
            case 2: return 'ACTIVE';
            case 1: return 'INIT';
            case 0: return 'STOPPED';
            default: return 'UNKNOWN';
        }
    };

    const uptime = new Date(metrics.uptime_seconds * 1000).toISOString().substr(11, 8);

    return (
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5 shadow-lg">
            <div className="flex items-center justify-between mb-4">
                <h2 className="text-sm font-semibold text-slate-400 uppercase tracking-wider flex items-center gap-2">
                    <Cpu className="w-4 h-4" /> System Health
                </h2>
                <div className="text-xs font-mono text-slate-500 flex items-center gap-1">
                    <Clock className="w-3 h-3" /> {uptime}
                </div>
            </div>

            <div className="space-y-3">
                <StatusRow label="Data Engine" state={metrics.data_engine_state} getStatusColor={getStatusColor} getStatusText={getStatusText} />
                <StatusRow label="Execution Engine" state={metrics.exec_engine_state} getStatusColor={getStatusColor} getStatusText={getStatusText} />
                <StatusRow label="Risk Engine" state={metrics.risk_engine_state} getStatusColor={getStatusColor} getStatusText={getStatusText} />
                <StatusRow label="Strategy" state={metrics.risk_engine_state} getStatusColor={getStatusColor} getStatusText={getStatusText} />
            </div>
        </div>
    );
}

function StatusRow({ label, state, getStatusColor, getStatusText }: any) {
    return (
        <div className="flex items-center justify-between bg-slate-950/50 p-3 rounded-lg border border-slate-800/50">
            <span className="text-slate-300 font-medium">{label}</span>
            <span className={cn("px-2 py-1 rounded text-xs font-bold border", getStatusColor(state))}>
                {getStatusText(state)}
            </span>
        </div>
    );
}
