#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>

namespace npcTrading
{
  namespace monitoring
  {

    // ============================================================================
    // System Metrics - Thread-safe metrics structure for monitoring
    // ============================================================================

    /**
     * @brief Atomic metrics structure for real-time system monitoring
     *
     * All fields are atomic to allow lock-free reading from the dashboard
     * while the trading system writes updates. This structure is designed
     * to be placed in shared memory for cross-process access.
     */
    struct SystemMetrics
    {
      // ========================================================================
      // MessageBus Metrics
      // ========================================================================
      std::atomic<uint64_t> messages_processed{ 0 };
      std::atomic<size_t> queue_size{ 0 };
      std::atomic<uint64_t> messages_per_second{ 0 };

      // ========================================================================
      // DataEngine / Market Data Metrics
      // ========================================================================
      std::atomic<uint64_t> quotes_received{ 0 };
      std::atomic<uint64_t> trades_received{ 0 };
      std::atomic<uint64_t> bars_received{ 0 };
      std::atomic<uint64_t> order_book_updates{ 0 };
      std::atomic<bool> websocket_connected{ false };
      std::atomic<int64_t> last_quote_timestamp_ms{ 0 };
      std::atomic<int64_t> last_trade_timestamp_ms{ 0 };

      // ========================================================================
      // ExecutionEngine Metrics
      // ========================================================================
      std::atomic<uint64_t> orders_submitted{ 0 };
      std::atomic<uint64_t> orders_accepted{ 0 };
      std::atomic<uint64_t> orders_filled{ 0 };
      std::atomic<uint64_t> orders_rejected{ 0 };
      std::atomic<uint64_t> orders_cancelled{ 0 };
      std::atomic<uint64_t> orders_pending{ 0 };

      // ========================================================================
      // RiskEngine Metrics
      // ========================================================================
      std::atomic<uint64_t> risk_checks_passed{ 0 };
      std::atomic<uint64_t> risk_checks_failed{ 0 };
      std::atomic<int> trading_state{ 2 };  // 0=HALTED, 1=REDUCING, 2=ACTIVE

      // ========================================================================
      // Component States (matching ComponentState enum)
      // ========================================================================
      // 0=PRE_INITIALIZED, 1=INITIALIZED, 2=READY, 3=RUNNING, 4=STOPPED, 5=DISPOSED
      std::atomic<int> data_engine_state{ 0 };
      std::atomic<int> exec_engine_state{ 0 };
      std::atomic<int> risk_engine_state{ 0 };
      std::atomic<int> strategy_state{ 0 };

      // ========================================================================
      // System Health
      // ========================================================================
      std::atomic<int64_t> last_heartbeat_ms{ 0 };
      std::atomic<uint64_t> uptime_seconds{ 0 };
      std::atomic<int64_t> start_time_ms{ 0 };

      // ========================================================================
      // Position & PnL Summary
      // ========================================================================
      std::atomic<int64_t> total_unrealized_pnl_cents{ 0 };  // In cents to avoid float atomics
      std::atomic<int64_t> total_realized_pnl_cents{ 0 };
      std::atomic<uint32_t> open_positions_count{ 0 };

      // ========================================================================
      // Methods
      // ========================================================================

      void reset()
      {
        messages_processed.store(0);
        queue_size.store(0);
        messages_per_second.store(0);
        quotes_received.store(0);
        trades_received.store(0);
        bars_received.store(0);
        order_book_updates.store(0);
        websocket_connected.store(false);
        last_quote_timestamp_ms.store(0);
        last_trade_timestamp_ms.store(0);
        orders_submitted.store(0);
        orders_accepted.store(0);
        orders_filled.store(0);
        orders_rejected.store(0);
        orders_cancelled.store(0);
        orders_pending.store(0);
        risk_checks_passed.store(0);
        risk_checks_failed.store(0);
        trading_state.store(2);
        data_engine_state.store(0);
        exec_engine_state.store(0);
        risk_engine_state.store(0);
        strategy_state.store(0);
        last_heartbeat_ms.store(0);
        uptime_seconds.store(0);
        start_time_ms.store(0);
        total_unrealized_pnl_cents.store(0);
        total_realized_pnl_cents.store(0);
        open_positions_count.store(0);
      }

      void update_heartbeat()
      {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        last_heartbeat_ms.store(ms);

        if (start_time_ms.load() > 0)
        {
          uptime_seconds.store((ms - start_time_ms.load()) / 1000);
        }
      }

      void mark_started()
      {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        start_time_ms.store(ms);
        last_heartbeat_ms.store(ms);
      }
    };

    // ============================================================================
    // Trading State Names (for display)
    // ============================================================================

    inline const char* trading_state_name(int state)
    {
      switch (state)
      {
        case 0: return "HALTED";
        case 1: return "REDUCING";
        case 2: return "ACTIVE";
        default: return "UNKNOWN";
      }
    }

    // ============================================================================
    // Component State Names (for display)
    // ============================================================================

    inline const char* component_state_name(int state)
    {
      switch (state)
      {
        case 0: return "PRE_INITIALIZED";
        case 1: return "INITIALIZED";
        case 2: return "READY";
        case 3: return "RUNNING";
        case 4: return "STOPPED";
        case 5: return "DISPOSED";
        default: return "UNKNOWN";
      }
    }

  }  // namespace monitoring
}  // namespace npcTrading
