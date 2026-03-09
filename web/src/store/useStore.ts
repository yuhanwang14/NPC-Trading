import { create } from 'zustand';
import { type SystemMetrics, defaultMetrics } from '../types';

interface DashboardState {
  metrics: SystemMetrics;
  connected: boolean;
  lastUpdate: number;
  connect: () => void;
  disconnect: () => void;
}

export const useStore = create<DashboardState>((set) => {
  let ws: WebSocket | null = null;
  let reconnectTimer: any = null;

  const connect = () => {
    if (ws) return;

    // Use current host or hardcoded for dev (if running via Vite on 5173, backend is on 8080)
    // In production (served by C++), host is same origin
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const host = window.location.port === '5173' ? 'localhost:8080' : window.location.host;
    const url = `${protocol}//${host}/ws`;

    console.log(`Connecting to WebSocket: ${url}`);
    ws = new WebSocket(url);

    ws.onopen = () => {
      console.log('WebSocket connected');
      set({ connected: true });
    };

    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        set({ 
          metrics: data,
          lastUpdate: Date.now()
        });
      } catch (e) {
        console.error('Failed to parse metrics:', e);
      }
    };

    ws.onclose = () => {
      console.log('WebSocket disconnected');
      set({ connected: false });
      ws = null;
      // Reconnect after 2 seconds
      reconnectTimer = setTimeout(connect, 2000);
    };

    ws.onerror = (error) => {
      console.error('WebSocket error:', error);
      ws?.close();
    };
  };

  const disconnect = () => {
    if (reconnectTimer) clearTimeout(reconnectTimer);
    ws?.close();
    ws = null;
    set({ connected: false });
  };

  return {
    metrics: defaultMetrics,
    connected: false,
    lastUpdate: 0,
    connect,
    disconnect
  };
});
