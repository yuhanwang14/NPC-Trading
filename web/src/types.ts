export interface SystemMetrics {
  // MessageBus metrics
  messages_processed: number;
  queue_size: number;
  
  // DataEngine metrics  
  quotes_received: number;
  trades_received: number;
  bars_received: number;
  websocket_connected: boolean;
  
  // ExecutionEngine metrics
  orders_submitted: number;
  orders_filled: number;
  orders_rejected: number;
  
  // RiskEngine metrics
  orders_approved: number;
  orders_denied: number;
  trading_state: number; // 0=HALTED, 1=REDUCING, 2=ACTIVE
  
  // Component states
  data_engine_state: number;
  exec_engine_state: number;
  risk_engine_state: number;
  
  // System health
  last_heartbeat_ms: number;
  uptime_seconds: number;
}

export const defaultMetrics: SystemMetrics = {
  messages_processed: 0,
  queue_size: 0,
  quotes_received: 0,
  trades_received: 0,
  bars_received: 0,
  websocket_connected: false,
  orders_submitted: 0,
  orders_filled: 0,
  orders_rejected: 0,
  orders_approved: 0,
  orders_denied: 0,
  trading_state: 0,
  data_engine_state: 0,
  exec_engine_state: 0,
  risk_engine_state: 0,
  last_heartbeat_ms: 0,
  uptime_seconds: 0
};
