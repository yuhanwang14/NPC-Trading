#pragma once

#include "common.hpp"
#include "component.hpp"
#include "model.hpp"
#include "message_bus.hpp"
#include <memory>

namespace npcTrading {

// ============================================================================
// Trading Commands
// ============================================================================

class TradingCommand : public Message {
public:
    virtual ~TradingCommand() = default;
};

class SubmitOrder : public TradingCommand {
public:
    explicit SubmitOrder(std::shared_ptr<Order> order) : order_(std::move(order)) {}
    
    Order* order() const { return order_.get(); }
    std::shared_ptr<Order> order_ptr() const { return order_; }
    
    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "SubmitOrder"; }
    
private:
    std::shared_ptr<Order> order_;
};

class ModifyOrder : public TradingCommand {
public:
    ModifyOrder(std::shared_ptr<Order> order, Quantity new_quantity, Price new_price)
        : order_(std::move(order)), new_quantity_(new_quantity), new_price_(new_price) {}
    
    Order* order() const { return order_.get(); }
    std::shared_ptr<Order> order_ptr() const { return order_; }
    Quantity new_quantity() const { return new_quantity_; }
    Price new_price() const { return new_price_; }
    
    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "ModifyOrder"; }
    
private:
    std::shared_ptr<Order> order_;
    Quantity new_quantity_;
    Price new_price_;
};

class CancelOrder : public TradingCommand {
public:
    explicit CancelOrder(std::shared_ptr<Order> order) : order_(std::move(order)) {}
    
    Order* order() const { return order_.get(); }
    std::shared_ptr<Order> order_ptr() const { return order_; }
    
    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "CancelOrder"; }
    
private:
    std::shared_ptr<Order> order_;
};

class CancelAllOrders : public TradingCommand {
public:
    explicit CancelAllOrders(InstrumentId instrument_id = "")
        : instrument_id_(instrument_id) {}
    
    InstrumentId instrument_id() const { return instrument_id_; }
    
    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "CancelAllOrders"; }
    
private:
    InstrumentId instrument_id_;
};

// ============================================================================
// Order Events
// ============================================================================

class OrderEvent : public Message {
public:
    virtual ~OrderEvent() = default;
    
    virtual OrderId order_id() const = 0;
};

class OrderSubmitted : public OrderEvent {
public:
    explicit OrderSubmitted(std::shared_ptr<Order> order, Timestamp timestamp)
        : order_(std::move(order)), timestamp_(timestamp) {}
    
    OrderId order_id() const override { return order_ ? order_->order_id() : ""; }
    Order* order() const { return order_.get(); }
    std::shared_ptr<Order> order_ptr() const { return order_; }
    
    Timestamp timestamp() const override { return timestamp_; }
    std::string type() const override { return "OrderSubmitted"; }
    
private:
    std::shared_ptr<Order> order_;
    Timestamp timestamp_;
};

class OrderAccepted : public OrderEvent {
public:
    explicit OrderAccepted(std::shared_ptr<Order> order, Timestamp timestamp)
        : order_(std::move(order)), timestamp_(timestamp) {}
    
    OrderId order_id() const override { return order_ ? order_->order_id() : ""; }
    Order* order() const { return order_.get(); }
    std::shared_ptr<Order> order_ptr() const { return order_; }
    
    Timestamp timestamp() const override { return timestamp_; }
    std::string type() const override { return "OrderAccepted"; }
    
private:
    std::shared_ptr<Order> order_;
    Timestamp timestamp_;
};

class OrderRejected : public OrderEvent {
public:
    OrderRejected(std::shared_ptr<Order> order, const std::string& reason, Timestamp timestamp)
        : order_(std::move(order)), reason_(reason), timestamp_(timestamp) {}
    
    OrderId order_id() const override { return order_ ? order_->order_id() : ""; }
    Order* order() const { return order_.get(); }
    std::shared_ptr<Order> order_ptr() const { return order_; }
    std::string reason() const { return reason_; }
    
    Timestamp timestamp() const override { return timestamp_; }
    std::string type() const override { return "OrderRejected"; }
    
private:
    std::shared_ptr<Order> order_;
    std::string reason_;
    Timestamp timestamp_;
};

class OrderFilled : public OrderEvent {
public:
    OrderFilled(std::shared_ptr<Order> order, const Fill& fill, Timestamp timestamp)
        : order_(std::move(order)), fill_(fill), timestamp_(timestamp) {}
    
    OrderId order_id() const override { return order_ ? order_->order_id() : ""; }
    Order* order() const { return order_.get(); }
    std::shared_ptr<Order> order_ptr() const { return order_; }
    const Fill& fill() const { return fill_; }
    
    Timestamp timestamp() const override { return timestamp_; }
    std::string type() const override { return "OrderFilled"; }
    
private:
    std::shared_ptr<Order> order_;
    Fill fill_;
    Timestamp timestamp_;
};

class OrderCanceled : public OrderEvent {
public:
    explicit OrderCanceled(std::shared_ptr<Order> order, Timestamp timestamp)
        : order_(std::move(order)), timestamp_(timestamp) {}
    
    OrderId order_id() const override { return order_ ? order_->order_id() : ""; }
    Order* order() const { return order_.get(); }
    std::shared_ptr<Order> order_ptr() const { return order_; }
    
    Timestamp timestamp() const override { return timestamp_; }
    std::string type() const override { return "OrderCanceled"; }
    
private:
    std::shared_ptr<Order> order_;
    Timestamp timestamp_;
};

class OrderDenied : public OrderEvent {
public:
    OrderDenied(std::shared_ptr<Order> order, const std::string& reason, Timestamp timestamp)
        : order_(std::move(order)), reason_(reason), timestamp_(timestamp) {}
    
    OrderId order_id() const override { return order_ ? order_->order_id() : ""; }
    Order* order() const { return order_.get(); }
    std::shared_ptr<Order> order_ptr() const { return order_; }
    std::string reason() const { return reason_; }
    
    Timestamp timestamp() const override { return timestamp_; }
    std::string type() const override { return "OrderDenied"; }
    
private:
    std::shared_ptr<Order> order_;
    std::string reason_;
    Timestamp timestamp_;
};

// ============================================================================
// ExecutionClient Interface
// ============================================================================

/**
 * @brief Abstract interface for exchange execution adapters
 */
class ExecutionClient {
public:
    ExecutionClient(ClientId client_id, VenueId venue, AccountId account_id)
        : client_id_(client_id), venue_(venue), account_id_(account_id) {}
    
    virtual ~ExecutionClient() = default;
    
    ClientId client_id() const { return client_id_; }
    VenueId venue() const { return venue_; }
    AccountId account_id() const { return account_id_; }
    
    // Order operations
    virtual void submit_order(Order* order) = 0;
    virtual void modify_order(Order* order, Quantity new_quantity, Price new_price) = 0;
    virtual void cancel_order(Order* order) = 0;
    virtual void query_order(const OrderId& order_id) = 0;
    
    // Connection management
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
    
protected:
    ClientId client_id_;
    VenueId venue_;
    AccountId account_id_;
};

// ============================================================================
// ExecutionEngine Configuration
// ============================================================================

struct ExecEngineConfig {
    bool allow_cash_positions = true;
    OmsType oms_type = OmsType::NETTING;
};

struct LiveExecEngineConfig : public ExecEngineConfig {
    bool reconciliation_enabled = true;
    int reconciliation_interval_ms = 60000;
    bool inflight_check_enabled = true;
    int inflight_check_interval_ms = 5000;
};

// ============================================================================
// ExecutionEngine
// ============================================================================

/**
 * @brief Central order management and execution orchestration
 * 
 * Endpoints:
 * - ExecEngine.execute: Execute trading commands
 * - ExecEngine.process: Process order events
 */
class ExecutionEngine : public Component {
public:
    ExecutionEngine(MessageBus* msgbus, Cache* cache, Clock* clock,
                    const ExecEngineConfig& config = ExecEngineConfig());
    
    ~ExecutionEngine() override = default;
    
    // ========================================================================
    // Component Lifecycle
    // ========================================================================
    
protected:
    void on_initialize() override;
    void on_start() override;
    void on_stop() override;
    
    // ========================================================================
    // Client Management
    // ========================================================================
    
    void register_client(std::shared_ptr<ExecutionClient> client);
    ExecutionClient* get_client(const ClientId& client_id) const;
    ExecutionClient* get_client_for_venue(const VenueId& venue) const;
    
    // ========================================================================
    // Message Handlers
    // ========================================================================
    
    void handle_execute(const std::shared_ptr<Message>& msg);
    void handle_process(const std::shared_ptr<Message>& msg);
    
private:
    ExecEngineConfig config_;
    
    // Client routing
    std::unordered_map<ClientId, std::shared_ptr<ExecutionClient>> clients_;
    std::unordered_map<VenueId, ClientId> venue_to_client_;
    
    // Position management
    void handle_fill(const OrderFilled& event);
    void update_position(Order* order, const Fill& fill);
    PositionId generate_position_id(Order* order);
    
    // TODO: Add portfolio tracking
    // TODO: Add OMS logic (NETTING vs HEDGING)
};

// ============================================================================
// LiveExecutionEngine
// ============================================================================

class LiveExecutionEngine : public ExecutionEngine {
public:
    LiveExecutionEngine(MessageBus* msgbus, Cache* cache, Clock* clock,
                        const LiveExecEngineConfig& config);
    
    ~LiveExecutionEngine() override = default;

private:
    void on_start() override;
    void on_stop() override;

    LiveExecEngineConfig live_config_;
    
    // TODO: Add reconciliation logic
    // TODO: Add in-flight order tracking
};

} // namespace npcTrading
