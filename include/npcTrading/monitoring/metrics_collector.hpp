#pragma once

#include <functional>
#include <memory>
#include <string>

#include "system_metrics.hpp"

namespace npcTrading
{
  namespace monitoring
  {

    // ============================================================================
    // Shared Memory Configuration
    // ============================================================================

    constexpr const char* DEFAULT_SHM_NAME = "/npctrading_metrics";
    constexpr size_t SHM_SIZE = sizeof(SystemMetrics);

    // ============================================================================
    // MetricsCollector - Singleton for collecting and publishing system metrics
    // ============================================================================

    /**
     * @brief Singleton metrics collector with shared memory support
     *
     * The MetricsCollector provides:
     * - Thread-safe metric updates from any component
     * - Shared memory access for external dashboard processes
     * - Convenience methods for incrementing counters
     *
     * Usage in trading components:
     *   MetricsCollector::instance().increment_messages_processed();
     *   MetricsCollector::instance().set_websocket_connected(true);
     *
     * Usage in dashboard:
     *   auto* metrics = MetricsCollector::read_shared_memory();
     */
    class MetricsCollector
    {
     public:
      // ========================================================================
      // Singleton Access
      // ========================================================================

      /**
       * @brief Get the singleton instance
       * @param use_shared_memory If true (default), creates shared memory segment
       */
      static MetricsCollector& instance(bool use_shared_memory = true);

      // Non-copyable
      MetricsCollector(const MetricsCollector&) = delete;
      MetricsCollector& operator=(const MetricsCollector&) = delete;

      // ========================================================================
      // Lifecycle
      // ========================================================================

      /**
       * @brief Initialize the collector and shared memory
       */
      void initialize();

      /**
       * @brief Shutdown and cleanup shared memory
       */
      void shutdown();

      /**
       * @brief Get raw metrics pointer (for dashboard reading)
       */
      SystemMetrics* metrics()
      {
        return metrics_;
      }
      const SystemMetrics* metrics() const
      {
        return metrics_;
      }

      // ========================================================================
      // Static Shared Memory Access (for external processes)
      // ========================================================================

      /**
       * @brief Open existing shared memory segment (read-only)
       * @return Pointer to metrics or nullptr if not available
       */
      static SystemMetrics* read_shared_memory(const std::string& name = DEFAULT_SHM_NAME);

      /**
       * @brief Close shared memory handle
       */
      static void close_shared_memory(SystemMetrics* ptr);

      // ========================================================================
      // MessageBus Metrics
      // ========================================================================

      void increment_messages_processed()
      {
        metrics_->messages_processed.fetch_add(1, std::memory_order_relaxed);
      }

      void set_queue_size(size_t size)
      {
        metrics_->queue_size.store(size, std::memory_order_relaxed);
      }

      void set_messages_per_second(uint64_t rate)
      {
        metrics_->messages_per_second.store(rate, std::memory_order_relaxed);
      }

      // ========================================================================
      // Market Data Metrics
      // ========================================================================

      void increment_quotes_received()
      {
        metrics_->quotes_received.fetch_add(1, std::memory_order_relaxed);
        update_last_quote_timestamp();
      }

      void increment_trades_received()
      {
        metrics_->trades_received.fetch_add(1, std::memory_order_relaxed);
        update_last_trade_timestamp();
      }

      void increment_bars_received()
      {
        metrics_->bars_received.fetch_add(1, std::memory_order_relaxed);
      }

      void increment_order_book_updates()
      {
        metrics_->order_book_updates.fetch_add(1, std::memory_order_relaxed);
      }

      void set_websocket_connected(bool connected)
      {
        metrics_->websocket_connected.store(connected, std::memory_order_relaxed);
      }

      // ========================================================================
      // Execution Metrics
      // ========================================================================

      void increment_orders_submitted()
      {
        metrics_->orders_submitted.fetch_add(1, std::memory_order_relaxed);
      }

      void increment_orders_accepted()
      {
        metrics_->orders_accepted.fetch_add(1, std::memory_order_relaxed);
      }

      void increment_orders_filled()
      {
        metrics_->orders_filled.fetch_add(1, std::memory_order_relaxed);
      }

      void increment_orders_rejected()
      {
        metrics_->orders_rejected.fetch_add(1, std::memory_order_relaxed);
      }

      void increment_orders_cancelled()
      {
        metrics_->orders_cancelled.fetch_add(1, std::memory_order_relaxed);
      }

      void set_orders_pending(uint64_t count)
      {
        metrics_->orders_pending.store(count, std::memory_order_relaxed);
      }

      // ========================================================================
      // Risk Metrics
      // ========================================================================

      void increment_risk_checks_passed()
      {
        metrics_->risk_checks_passed.fetch_add(1, std::memory_order_relaxed);
      }

      void increment_risk_checks_failed()
      {
        metrics_->risk_checks_failed.fetch_add(1, std::memory_order_relaxed);
      }

      void set_trading_state(int state)
      {
        metrics_->trading_state.store(state, std::memory_order_relaxed);
      }

      // ========================================================================
      // Component State Updates
      // ========================================================================

      void set_data_engine_state(int state)
      {
        metrics_->data_engine_state.store(state, std::memory_order_relaxed);
      }

      void set_exec_engine_state(int state)
      {
        metrics_->exec_engine_state.store(state, std::memory_order_relaxed);
      }

      void set_risk_engine_state(int state)
      {
        metrics_->risk_engine_state.store(state, std::memory_order_relaxed);
      }

      void set_strategy_state(int state)
      {
        metrics_->strategy_state.store(state, std::memory_order_relaxed);
      }

      // ========================================================================
      // System Health
      // ========================================================================

      void heartbeat()
      {
        metrics_->update_heartbeat();
      }

      void mark_started()
      {
        metrics_->mark_started();
      }

      // ========================================================================
      // Position & PnL
      // ========================================================================

      void set_unrealized_pnl(double pnl)
      {
        metrics_->total_unrealized_pnl_cents.store(static_cast<int64_t>(pnl * 100), std::memory_order_relaxed);
      }

      void set_realized_pnl(double pnl)
      {
        metrics_->total_realized_pnl_cents.store(static_cast<int64_t>(pnl * 100), std::memory_order_relaxed);
      }

      void set_open_positions_count(uint32_t count)
      {
        metrics_->open_positions_count.store(count, std::memory_order_relaxed);
      }

     private:
      MetricsCollector(bool use_shared_memory);
      ~MetricsCollector();

      void init_shared_memory();
      void cleanup_shared_memory();

      void update_last_quote_timestamp()
      {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        metrics_->last_quote_timestamp_ms.store(ms, std::memory_order_relaxed);
      }

      void update_last_trade_timestamp()
      {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        metrics_->last_trade_timestamp_ms.store(ms, std::memory_order_relaxed);
      }

      bool use_shared_memory_;
      int shm_fd_;
      SystemMetrics* metrics_;
      std::unique_ptr<SystemMetrics> local_metrics_;  // Fallback when not using shm
    };

  }  // namespace monitoring
}  // namespace npcTrading
