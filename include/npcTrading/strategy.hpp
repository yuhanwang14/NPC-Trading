#pragma once

#include "common.hpp"
#include "component.hpp"
#include "cache.hpp"
#include "model.hpp"
#include "market_data.hpp"
#include "execution_engine.hpp"
#include <memory>

namespace npcTrading {

// ============================================================================
// Actor - Base class for all strategy components
// ============================================================================

/**
 * @brief Base class for strategies, algorithms, and other active components
 * 
 * Actor provides:
 * - Data subscription management
 * - Event handling infrastructure
 * - Access to cache, clock, and message bus
 */
class Actor : public Component {
public:
    Actor(const std::string& actor_id,
          MessageBus* msgbus,
          Cache* cache,
          Clock* clock);
    
    virtual ~Actor() = default;
    
    // ========================================================================
    // Lifecycle Callbacks
    // ========================================================================
    
    /**
     * @brief Called when actor starts
     * Override to perform initialization
     */
    virtual void on_start() {}
    
    /**
     * @brief Called when actor stops
     * Override to perform cleanup
     */
    virtual void on_stop() {}
    
    // ========================================================================
    // Market Data Callbacks
    // ========================================================================
    
    /**
     * @brief Called when a bar is received
     */
    virtual void on_bar(const Bar& bar) {}
    
    /**
     * @brief Called when a quote tick is received
     */
    virtual void on_quote(const QuoteTick& quote) {}
    
    /**
     * @brief Called when a trade tick is received
     */
    virtual void on_trade(const TradeTick& trade) {}
    
    /**
     * @brief Called when order book update is received
     */
    virtual void on_order_book(const OrderBook& book) {}
    
    // ========================================================================
    // Event Callbacks
    // ========================================================================
    
    /**
     * @brief Called when any event is received
     * Override for general event handling
     */
    virtual void on_event(const std::shared_ptr<Message>& event) {}
    
    // ========================================================================
    // Subscription Management
    // ========================================================================
    
    /**
     * @brief Subscribe to bars
     */
    void subscribe_bars(const BarType& bar_type);
    
    /**
     * @brief Unsubscribe from bars
     */
    void unsubscribe_bars(const BarType& bar_type);
    
    /**
     * @brief Subscribe to quotes
     */
    void subscribe_quotes(const InstrumentId& instrument_id);
    
    /**
     * @brief Unsubscribe from quotes
     */
    void unsubscribe_quotes(const InstrumentId& instrument_id);
    
    /**
     * @brief Subscribe to trades
     */
    void subscribe_trades(const InstrumentId& instrument_id);
    
    /**
     * @brief Unsubscribe from trades
     */
    void unsubscribe_trades(const InstrumentId& instrument_id);
    
    /**
     * @brief Subscribe to order book
     */
    void subscribe_order_book(const InstrumentId& instrument_id, int depth = 10);
    
    /**
     * @brief Unsubscribe from order book
     */
    void unsubscribe_order_book(const InstrumentId& instrument_id);
    
protected:
    // Message handler registration
    void register_event_handler(const std::string& endpoint);
    
    // Internal event dispatcher
    void handle_message(const std::shared_ptr<Message>& msg);
};

// ============================================================================
// Strategy Configuration
// ============================================================================

struct StrategyConfig {
    StrategyId strategy_id;
    OmsType oms_type = OmsType::NETTING;
    bool manage_positions = true;
    
    // Add strategy-specific configuration here
};

// ============================================================================
// Strategy - User-defined trading logic
// ============================================================================

/**
 * @brief Base class for trading strategies
 * 
 * Provides order submission and position management on top of Actor.
 */
class Strategy : public Actor {
public:
    Strategy(const StrategyConfig& config,
             MessageBus* msgbus,
             Cache* cache,
             Clock* clock);
    
    virtual ~Strategy() = default;
    
    StrategyId strategy_id() const { return config_.strategy_id; }
    
    // ========================================================================
    // Order Submission
    // ========================================================================
    
    /**
     * @brief Submit a single order
     */
    void submit_order(Order* order);
    
    /**
     * @brief Submit multiple orders atomically
     */
    void submit_order_list(const std::vector<Order*>& orders);
    
    /**
     * @brief Submit a market order
     */
    void submit_market_order(const InstrumentId& instrument_id,
                            OrderSide side,
                            Quantity quantity);
    
    /**
     * @brief Submit a limit order
     */
    void submit_limit_order(const InstrumentId& instrument_id,
                           OrderSide side,
                           Quantity quantity,
                           Price price);
    
    // ========================================================================
    // Order Management
    // ========================================================================
    
    /**
     * @brief Modify an existing order
     */
    void modify_order(Order* order, Quantity new_quantity, Price new_price);
    
    /**
     * @brief Cancel an order
     */
    void cancel_order(Order* order);
    
    /**
     * @brief Cancel all orders for an instrument
     */
    void cancel_all_orders(const InstrumentId& instrument_id = "");
    
    // ========================================================================
    // Position Queries
    // ========================================================================
    
    /**
     * @brief Get all open positions for this strategy
     */
    std::vector<const Position*> positions_open() const;
    
    /**
     * @brief Get position for a specific instrument
     */
    const Position* position_for(const InstrumentId& instrument_id) const;
    
    /**
     * @brief Check if strategy has any open position
     */
    bool has_position() const;
    
    /**
     * @brief Check if strategy has position for instrument
     */
    bool has_position(const InstrumentId& instrument_id) const;
    
    // ========================================================================
    // Order Callbacks
    // ========================================================================
    
    /**
     * @brief Called when order is submitted to risk engine
     */
    virtual void on_order_submitted(const Order& order) {}
    
    /**
     * @brief Called when order is accepted by exchange
     */
    virtual void on_order_accepted(const Order& order) {}
    
    /**
     * @brief Called when order is rejected
     */
    virtual void on_order_rejected(const Order& order, const std::string& reason) {}
    
    /**
     * @brief Called when order is denied by risk engine
     */
    virtual void on_order_denied(const Order& order, const std::string& reason) {}
    
    /**
     * @brief Called when order is filled (fully or partially)
     */
    virtual void on_order_filled(const Order& order, const Fill& fill) {}
    
    /**
     * @brief Called when order is canceled
     */
    virtual void on_order_canceled(const Order& order) {}
    
    // Override event handler to dispatch to specific callbacks
    void on_event(const std::shared_ptr<Message>& event) override;
    
protected:
    StrategyConfig config_;
    
    // Order ID generation
    OrderId generate_order_id();
    
private:
    int order_counter_ = 0;
};

// ============================================================================
// ExecAlgorithm - Complex execution algorithms
// ============================================================================

/*
 * @brief Base class for execution algorithms (TWAP, VWAP, etc.)
 * 
 * Execution algorithms manage parent orders and spawn child orders.
*/
class ExecAlgorithm : public Actor {
public:
    ExecAlgorithm(const std::string& algo_id,
                  MessageBus* msgbus,
                  Cache* cache,
                  Clock* clock);
    
    virtual ~ExecAlgorithm() = default;
    
    // ========================================================================
    // Child Order Spawning
    // ========================================================================
    
    /**
     * @brief Spawn a market order
     */
    void spawn_market_order(const InstrumentId& instrument_id,
                           OrderSide side,
                           Quantity quantity,
                           TimeInForce tif = TimeInForce::GTC);
    
    /**
     * @brief Spawn a limit order
     */
    void spawn_limit_order(const InstrumentId& instrument_id,
                          OrderSide side,
                          Quantity quantity,
                          Price price,
                          TimeInForce tif = TimeInForce::GTC);
    
    // ========================================================================
    // Target Order Management
    // ========================================================================
    
    /**
     * @brief Subscribe to a parent order
     */
    void subscribe_to_order(const OrderId& order_id);
    
    /**
     * @brief Callback when target order is filled
     */
    virtual void on_target_order_filled(const Order& order, const Fill& fill) {}
    
protected:
    int child_order_counter_ = 0;
};

} // namespace npcTrading
