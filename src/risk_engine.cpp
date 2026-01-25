#include "npcTrading/risk_engine.hpp"

#include "npcTrading/monitoring/metrics_collector.hpp"

namespace npcTrading
{

  RiskEngine::RiskEngine(MessageBus* msgbus, Cache* cache, Clock* clock, const RiskEngineConfig& config)
      : Component("RiskEngine", msgbus, cache, clock),
        config_(config),
        trading_state_(config.initial_state)
  {
  }

  void RiskEngine::on_initialize()
  {
    // Register message handlers
    msgbus_->register_handler(
        Endpoints::RISK_ENGINE_EXECUTE, [this](const std::shared_ptr<Message>& msg) { handle_execute(msg); });

    msgbus_->register_handler(
        Endpoints::RISK_ENGINE_PROCESS, [this](const std::shared_ptr<Message>& msg) { handle_process(msg); });

    log_info("RiskEngine initialized (bypass_risk=" + std::string(config_.bypass_risk ? "true" : "false") + ")");
  }

  void RiskEngine::on_start()
  {
    log_info("RiskEngine started with trading_state=" + to_string(trading_state_));
  }

  void RiskEngine::on_stop()
  {
    log_info("RiskEngine stopped");
  }

  void RiskEngine::set_trading_state(TradingState state)
  {
    trading_state_ = state;
    monitoring::MetricsCollector::instance().set_trading_state(static_cast<int>(state));
    log_info("Trading state changed to: " + to_string(state));
  }

  void RiskEngine::handle_execute(const std::shared_ptr<Message>& msg)
  {
    std::string rejection_reason;

    const TradingCommand* command = dynamic_cast<const TradingCommand*>(msg.get());
    if (!command)
    {
      log_error("Received non-TradingCommand message");
      return;
    }

    if (validate_command(command, rejection_reason))
    {
      monitoring::MetricsCollector::instance().increment_risk_checks_passed();
      forward_to_execution(msg);
    }
    else
    {
      // Extract order from command and deny
      monitoring::MetricsCollector::instance().increment_risk_checks_failed();
      if (auto submit = dynamic_cast<const SubmitOrder*>(command))
      {
        deny_order(submit->order(), rejection_reason);
      }
      else
      {
        log_warning("Command denied (non-submit): " + rejection_reason);
      }
    }
  }

  void RiskEngine::handle_process(const std::shared_ptr<Message>& msg)
  {
    // TODO: Update risk state based on events
  }

  bool RiskEngine::validate_command(const TradingCommand* command, std::string& rejection_reason)
  {
    if (config_.bypass_risk)
    {
      return true;
    }

    // TODO: Implement full validation
    return true;
  }

  bool RiskEngine::validate_submit_order(const SubmitOrder* command, std::string& rejection_reason)
  {
    // TODO: Implement validation
    return true;
  }

  bool RiskEngine::validate_order_against_instrument(const Order* order, std::string& rejection_reason)
  {
    // TODO: Implement validation
    return true;
  }

  bool RiskEngine::check_price_precision(const Order* order, const Instrument* instrument, std::string& reason)
  {
    // TODO: Implement precision check
    return true;
  }

  bool RiskEngine::check_quantity_precision(const Order* order, const Instrument* instrument, std::string& reason)
  {
    // TODO: Implement precision check
    return true;
  }

  bool RiskEngine::check_quantity_limits(const Order* order, const Instrument* instrument, std::string& reason)
  {
    // TODO: Implement limit check
    return true;
  }

  bool RiskEngine::check_notional_limits(const Order* order, std::string& reason)
  {
    // TODO: Implement notional check
    return true;
  }

  bool RiskEngine::check_position_limits(const Order* order, std::string& reason)
  {
    // TODO: Implement position limit check
    return true;
  }

  bool RiskEngine::check_reduce_only(const Order* order, std::string& reason)
  {
    // TODO: Implement reduce-only check
    return true;
  }

  void RiskEngine::deny_order(Order* order, const std::string& reason)
  {
    if (!order)
      return;

    log_warning("Order denied: " + order->order_id() + " - " + reason);

    // Create OrderDenied event and send to ExecutionEngine for processing
    auto denied = std::make_shared<OrderDenied>(std::make_shared<Order>(*order), reason, std::chrono::system_clock::now());

    msgbus_->send(Endpoints::EXEC_ENGINE_PROCESS, denied);
  }

  void RiskEngine::forward_to_execution(const std::shared_ptr<Message>& msg)
  {
    msgbus_->send(Endpoints::EXEC_ENGINE_EXECUTE, msg);
  }

  // LiveRiskEngine implementation
  LiveRiskEngine::LiveRiskEngine(MessageBus* msgbus, Cache* cache, Clock* clock, const LiveRiskEngineConfig& config)
      : RiskEngine(msgbus, cache, clock, config),
        live_config_(config)
  {
  }

  void LiveRiskEngine::on_start()
  {
    RiskEngine::on_start();
    // TODO: Live-specific start logic
  }

  void LiveRiskEngine::on_stop()
  {
    RiskEngine::on_stop();
    // TODO: Live-specific stop logic
  }

}  // namespace npcTrading
