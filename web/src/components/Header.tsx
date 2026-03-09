import { Activity } from 'lucide-react';
import { useStore } from '../store/useStore';

export function Header() {
    const { connected } = useStore();

    return (
        <header className="bg-slate-900 border-b border-slate-800 p-4">
            <div className="container mx-auto flex items-center justify-between">
                <div className="flex items-center gap-3">
                    <div className="bg-blue-600 p-2 rounded-lg">
                        <Activity className="w-6 h-6 text-white" />
                    </div>
                    <h1 className="text-xl font-bold bg-gradient-to-r from-blue-400 to-cyan-300 bg-clip-text text-transparent">
                        npcTrading Dashboard
                    </h1>
                </div>

                <div className="flex items-center gap-3 bg-slate-800 px-4 py-2 rounded-full border border-slate-700">
                    <div className={`w-2.5 h-2.5 rounded-full ${connected ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`} />
                    <span className={`text-sm font-medium ${connected ? 'text-green-400' : 'text-red-400'}`}>
                        {connected ? 'Connected' : 'Disconnected'}
                    </span>
                </div>
            </div>
        </header>
    );
}
