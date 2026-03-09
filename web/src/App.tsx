import { useEffect } from 'react';
import { useStore } from './store/useStore';
import { Header } from './components/Header';
import { SystemStatus } from './components/SystemStatus';
import { MarketData } from './components/MarketData';
import { OrderFlow } from './components/OrderFlow';
import { RiskStatus } from './components/RiskStatus';
import { MessageBus } from './components/MessageBus';

function App() {
  const { connect, disconnect } = useStore();

  useEffect(() => {
    connect();
    return () => disconnect();
  }, [connect, disconnect]);

  return (
    <div className="min-h-screen bg-slate-950 text-slate-200 font-sans selection:bg-blue-500/30">
      <Header />

      <main className="container mx-auto p-4 md:p-6 lg:p-8">
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6">
          <SystemStatus />
          <MarketData />
          <OrderFlow />
          <RiskStatus />
          <MessageBus />

          {/* Placeholder for future PnL card */}
          <div className="bg-slate-900 border border-slate-800 rounded-xl p-5 shadow-lg opacity-50 border-dashed">
            <div className="h-full flex items-center justify-center text-slate-600 text-sm">
              Positions & PnL (Coming Soon)
            </div>
          </div>
        </div>
      </main>
    </div>
  );
}

export default App;
