import { useStore } from '../store/useStore';
import { MessageSquare, Zap } from 'lucide-react';
import { AreaChart, Area, ResponsiveContainer, YAxis } from 'recharts';

export function MessageBus() {
    const { metrics } = useStore();

    // Mock data for sparkline (in a real app, we'd keep history in store)
    const data = [
        { v: 10 }, { v: 15 }, { v: 12 }, { v: 20 }, { v: 25 }, { v: 18 }, { v: 30 }
    ];

    return (
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5 shadow-lg">
            <div className="flex items-center justify-between mb-4">
                <h2 className="text-sm font-semibold text-slate-400 uppercase tracking-wider flex items-center gap-2">
                    <MessageSquare className="w-4 h-4" /> Message Bus
                </h2>
                <Zap className="w-4 h-4 text-blue-400" />
            </div>

            <div className="grid grid-cols-2 gap-4 mb-4">
                <div>
                    <div className="text-xs text-slate-500 mb-1">Messages Processed</div>
                    <div className="text-2xl font-bold text-blue-400 font-mono">
                        {metrics.messages_processed.toLocaleString()}
                    </div>
                </div>
                <div>
                    <div className="text-xs text-slate-500 mb-1">Queue Size</div>
                    <div className="text-2xl font-bold text-purple-400 font-mono">
                        {metrics.queue_size.toLocaleString()}
                    </div>
                </div>
            </div>

            <div className="h-16 mt-2 opacity-50">
                <ResponsiveContainer width="100%" height="100%">
                    <AreaChart data={data}>
                        <YAxis hide domain={['dataMin', 'dataMax']} />
                        <Area type="monotone" dataKey="v" stroke="#3b82f6" fill="#3b82f6" fillOpacity={0.2} />
                    </AreaChart>
                </ResponsiveContainer>
            </div>
        </div>
    );
}
