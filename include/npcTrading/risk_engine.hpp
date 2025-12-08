#pragma once

#include "common.hpp"
#include "component.hpp"
#include "model.hpp"
#include "market_data.hpp"
#include "execution_engine.hpp"
#include "message_bus.hpp"

namespace npcTrading {

// ============================================================================
// RiskEngine Configuration
// ============================================================================

struct RiskEngineConfig {
    bool bypass_risk = false;  // For testing only
    bool allow_short_positions = true;
    Money max_order_notional = Money(100000.0, "USD");
    Money max_position_notional = Money(500000.0, "USD");
    double max_position_size = 1000.0;
    TradingState initial_state = TradingState::ACTIVE;
};

struct LiveRiskEngineConfig : public RiskEngineConfig {
    bool reconciliation_enabled = true;
    int reconciliation_interval_ms = 60000;
};

// ============================================================================
// Risk Rejection Reasons
// ============================================================================

namespace RiskRejection {
    constexpr const char* EXCEEDS_MAX_NOTIONAL = "Order exceeds maximum notional";
    constexpr const char* EXCEEDS_MAX_POSITION = "Position would exceed maximum size";
    constexpr const char* INVALID_PRICE_PRECISION = "Price does not match tick size";
    constexpr const char* INVALID_QTY_PRECISION = "Quantity does not match step size";
    constexpr const char* BELOW_MIN_QUANTITY = "Quantity below minimum";
    constexpr const char* ABOVE_MAX_QUANTITY = "Quantity above maximum";
    constexpr const char* TRADING_HALTED = "Trading is halted";
    constexpr const char* REDUCE_ONLY_VIOLATION = "Order does not reduce position";
    constexpr const char* SHORT_NOT_ALLOWED = "Short selling not allowed";
}

// ============================================================================
// RiskEngine
// ============================================================================

/**
 * @brief Pre-trade risk validation engine
 * 
 * Validates all trading commands before they reach ExecutionEngine.
 * 
 * Endpoints:
 * - RiskEngine.execute: Receive and validate trading commands
 * - RiskEngine.process: Update risk state based on events
 */
class RiskEngine : public Component {
public:
    RiskEngine(MessageBus* msgbus, Cache* cache, Clock* clock,
               const RiskEngineConfig& config = RiskEngineConfig());
    
    ~RiskEngine() override = default;
    
    // ========================================================================
    // Component Lifecycle
    // ========================================================================
    
protected:
    void on_initialize() override;
    void on_start() override;
    void on_stop() override;
    
    // ========================================================================
    // Trading State Management
    // ========================================================================
    
    TradingState trading_state() const { return trading_state_; }
    void set_trading_state(TradingState state);
    
    // ========================================================================
    // Message Handlers
    // ========================================================================
    
    void handle_execute(const std::shared_ptr<Message>& msg);
    void handle_process(const std::shared_ptr<Message>& msg);
    
private:
    RiskEngineConfig config_;
    TradingState trading_state_;
    
    // ========================================================================
    // Validation Methods
    // ========================================================================
    
    /**
     * @brief Validate a trading command
     * @return true if valid, false otherwise
     */
    bool validate_command(const TradingCommand* command, std::string& rejection_reason);
    
    /**
     * @brief Validate a submit order command
     */
    bool validate_submit_order(const SubmitOrder* command, std::string& rejection_reason);
    
    /**
     * @brief Validate order against instrument specification
     */
    bool validate_order_against_instrument(const Order* order, std::string& rejection_reason);
    
    /**
     * @brief Check price precision
     */
    bool check_price_precision(const Order* order, const Instrument* instrument, 
                               std::string& reason);
    
    /**
     * @brief Check quantity precision
     */
    bool check_quantity_precision(const Order* order, const Instrument* instrument,
                                  std::string& reason);
    
    /**
     * @brief Check quantity limits
     */
    bool check_quantity_limits(const Order* order, const Instrument* instrument,
                               std::string& reason);
    
    /**
     * @brief Check notional limits
     */
    bool check_notional_limits(const Order* order, std::string& reason);
    
    /**
     * @brief Check position size limits
     */
    bool check_position_limits(const Order* order, std::string& reason);
    
    /**
     * @brief Check if order reduces position (for REDUCING mode)
     */
    bool check_reduce_only(const Order* order, std::string& reason);
    
    /**
     * @brief Deny an order and send OrderDenied event
     */
    void deny_order(Order* order, const std::string& reason);
    
    /**
     * @brief Forward approved command to ExecutionEngine
     */
    void forward_to_execution(const std::shared_ptr<Message>& msg);
};

// ============================================================================
// LiveRiskEngine
// ============================================================================

class LiveRiskEngine : public RiskEngine {
public:
    LiveRiskEngine(MessageBus* msgbus, Cache* cache, Clock* clock,
                   const LiveRiskEngineConfig& config);
    
    ~LiveRiskEngine() override = default;

private:
    void on_start() override;
    void on_stop() override;

    LiveRiskEngineConfig live_config_;
    
    // TODO: Add periodic reconciliation
    // TODO: Add real-time risk metrics calculation
};

} // namespace npcTrading
