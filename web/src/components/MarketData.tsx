import { useStore } from '../store/useStore';
import { BarChart, Wifi, ArrowDown, Activity } from 'lucide-react';

export function MarketData() {
    const { metrics } = useStore();

    return (
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5 shadow-lg">
            <div className="flex items-center justify-between mb-4">
                <h2 className="text-sm font-semibold text-slate-400 uppercase tracking-wider flex items-center gap-2">
                    <BarChart className="w-4 h-4" /> Market Data
                </h2>
                <div className={`p-1.5 rounded-md ${metrics.websocket_connected ? 'bg-green-500/20 text-green-400' : 'bg-red-500/20 text-red-400'}`}>
                    <Wifi className="w-4 h-4" />
                </div>
            </div>

            <div className="grid grid-cols-2 gap-3">
                <MetricCard label="Quotes" value={metrics.quotes_received} icon={<Activity className="w-4 h-4 text-blue-400" />} />
                <MetricCard label="Trades" value={metrics.trades_received} icon={<ArrowDown className="w-4 h-4 text-purple-400" />} />
                <MetricCard label="Bars" value={metrics.bars_received} icon={<BarChart className="w-4 h-4 text-orange-400" />} />
                <MetricCard label="Updates" value={0} icon={<Activity className="w-4 h-4 text-cyan-400" />} />
            </div>
        </div>
    );
}

function MetricCard({ label, value, icon }: any) {
    return (
        <div className="bg-slate-950 p-3 rounded-lg border border-slate-800">
            <div className="flex items-center justify-between mb-1">
                <span className="text-xs text-slate-500">{label}</span>
                {icon}
            </div>
            <div className="text-xl font-bold text-slate-200 font-mono">
                {value.toLocaleString()}
            </div>
        </div>
    );
}
