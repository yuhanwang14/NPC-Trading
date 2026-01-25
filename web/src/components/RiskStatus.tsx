import { useStore } from '../store/useStore';
import { ShieldAlert, AlertTriangle } from 'lucide-react';

export function RiskStatus() {
    const { metrics } = useStore();

    const getRiskState = (state: number) => {
        switch (state) {
            case 2: return { text: 'ACTIVE', color: 'text-green-400 border-green-500/30 bg-green-500/10' };
            case 1: return { text: 'REDUCING', color: 'text-yellow-400 border-yellow-500/30 bg-yellow-500/10' };
            case 0: return { text: 'HALTED', color: 'text-red-400 border-red-500/30 bg-red-500/10' };
            default: return { text: 'UNKNOWN', color: 'text-slate-400 border-slate-500/30 bg-slate-500/10' };
        }
    };

    const risk = getRiskState(metrics.trading_state);

    return (
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5 shadow-lg">
            <div className="flex items-center justify-between mb-4">
                <h2 className="text-sm font-semibold text-slate-400 uppercase tracking-wider flex items-center gap-2">
                    <ShieldAlert className="w-4 h-4" /> Risk Status
                </h2>
                <AlertTriangle className="w-5 h-5 text-yellow-500" />
            </div>

            <div className="mb-6">
                <div className="text-xs text-slate-500 mb-2">Trading State</div>
                <div className={`text-center py-3 rounded-lg border font-bold tracking-widest ${risk.color}`}>
                    {risk.text}
                </div>
            </div>

            <div className="grid grid-cols-2 gap-3">
                <div className="bg-slate-950 p-3 rounded-lg border border-slate-800">
                    <div className="text-xs text-slate-500 mb-1">Checks Passed</div>
                    <div className="text-xl font-bold font-mono text-green-400">{metrics.orders_approved.toLocaleString()}</div>
                </div>
                <div className="bg-slate-950 p-3 rounded-lg border border-slate-800">
                    <div className="text-xs text-slate-500 mb-1">Checks Failed</div>
                    <div className="text-xl font-bold font-mono text-red-400">{metrics.orders_denied.toLocaleString()}</div>
                </div>
            </div>
        </div>
    );
}
