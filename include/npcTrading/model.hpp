#pragma once

#include "common.hpp"
#include <string>
#include <vector>

namespace npcTrading {

// ============================================================================
// Order
// ============================================================================

class Order {
public:
    Order(OrderId order_id,
          StrategyId strategy_id,
          InstrumentId instrument_id,
          ClientId client_id,
          OrderSide side,
          OrderType type,
          Quantity quantity,
          Price price = Price(0),
          TimeInForce time_in_force = TimeInForce::GTC,
          Timestamp timestamp = std::chrono::system_clock::now(),
          Price stop_price = Price(0),           // For STOP orders
          double trailing_delta = 0.0,           // For TRAILING_STOP (percent or absolute)
          Quantity iceberg_qty = Quantity(0))     // For ICEBERG orders (visible qty)
        : order_id_(order_id),
          strategy_id_(strategy_id),
          instrument_id_(instrument_id),
          client_id_(client_id),
          side_(side),
          type_(type),
          quantity_(quantity),
          price_(price),
          time_in_force_(time_in_force),
          filled_qty_(0.0),
          status_(OrderStatus::INITIALIZED),
          timestamp_(timestamp),
          stop_price_(stop_price),
          trailing_delta_(trailing_delta),
          iceberg_qty_(iceberg_qty),
          activation_price_(Price(0)),
          client_order_id_(""),
          exchange_order_id_("") {}
    
    // Getters
    OrderId order_id() const { return order_id_; }
    StrategyId strategy_id() const { return strategy_id_; }
    InstrumentId instrument_id() const { return instrument_id_; }
    ClientId client_id() const { return client_id_; }
    OrderSide side() const { return side_; }
    OrderType type() const { return type_; }
    Quantity quantity() const { return quantity_; }
    Price price() const { return price_; }
    TimeInForce time_in_force() const { return time_in_force_; }
    Quantity filled_qty() const { return filled_qty_; }
    OrderStatus status() const { return status_; }
    PositionId position_id() const { return position_id_; }
    Timestamp timestamp() const { return timestamp_; }
    
    // Extended getters for new order types
    Price stop_price() const { return stop_price_; }
    double trailing_delta() const { return trailing_delta_; }
    Quantity iceberg_qty() const { return iceberg_qty_; }
    Price activation_price() const { return activation_price_; }
    std::string client_order_id() const { return client_order_id_; }
    std::string exchange_order_id() const { return exchange_order_id_; }
    
    // Status checks
    bool is_open() const;
    bool is_closed() const;
    bool is_filled() const { return status_ == OrderStatus::FILLED; }
    bool is_stop_order() const { 
        return type_ == OrderType::STOP_MARKET || 
               type_ == OrderType::STOP_LIMIT ||
               type_ == OrderType::STOP_LOSS ||
               type_ == OrderType::TAKE_PROFIT ||
               type_ == OrderType::TAKE_PROFIT_LIMIT ||
               type_ == OrderType::TRAILING_STOP_MARKET;
    }
    bool is_conditional() const { return is_stop_order(); }
    
    // Modifications
    void set_status(OrderStatus status) { status_ = status; }
    void set_filled_qty(Quantity qty) { filled_qty_ = qty; }
    void set_position_id(const PositionId& position_id) { position_id_ = position_id; }
    void update_price(Price new_price) { price_ = new_price; }
    void update_quantity(Quantity new_quantity) { quantity_ = new_quantity; }
    void set_timestamp(Timestamp ts) { timestamp_ = ts; }
    void set_stop_price(Price stop_price) { stop_price_ = stop_price; }
    void set_activation_price(Price activation_price) { activation_price_ = activation_price; }
    void set_client_order_id(const std::string& id) { client_order_id_ = id; }
    void set_exchange_order_id(const std::string& id) { exchange_order_id_ = id; }
    
private:
    OrderId order_id_;
    StrategyId strategy_id_;
    InstrumentId instrument_id_;
    ClientId client_id_;
    OrderSide side_;
    OrderType type_;
    Quantity quantity_;
    Price price_;
    TimeInForce time_in_force_;
    Quantity filled_qty_;
    OrderStatus status_;
    PositionId position_id_;
    Timestamp timestamp_;
    
    // Extended fields for advanced order types
    Price stop_price_;              // Trigger price for stop orders
    double trailing_delta_;         // Trailing amount/percentage for trailing stops
    Quantity iceberg_qty_;          // Visible quantity for iceberg orders
    Price activation_price_;        // Actual activation price for triggered orders
    std::string client_order_id_;   // Client-generated unique order ID
    std::string exchange_order_id_; // Exchange-assigned order ID
};

// ============================================================================
// Fill (Execution Report)
// ============================================================================

class Fill {
public:
    Fill(OrderId order_id,
         InstrumentId instrument_id,
         Price price,
         Quantity quantity,
         OrderSide side,
         Timestamp timestamp)
        : order_id_(order_id),
          instrument_id_(instrument_id),
          price_(price),
          quantity_(quantity),
          side_(side),
          timestamp_(timestamp) {}
    
    OrderId order_id() const { return order_id_; }
    InstrumentId instrument_id() const { return instrument_id_; }
    Price price() const { return price_; }
    Quantity quantity() const { return quantity_; }
    OrderSide side() const { return side_; }
    Timestamp timestamp() const { return timestamp_; }
    
private:
    OrderId order_id_;
    InstrumentId instrument_id_;
    Price price_;
    Quantity quantity_;
    OrderSide side_;
    Timestamp timestamp_;
};

// ============================================================================
// Position
// ============================================================================

class Position {
public:
    Position(PositionId position_id,
             InstrumentId instrument_id,
             StrategyId strategy_id,
             Timestamp timestamp = std::chrono::system_clock::now())
        : position_id_(position_id),
          instrument_id_(instrument_id),
          strategy_id_(strategy_id),
          side_(PositionSide::FLAT),
          quantity_(0.0),
          entry_price_(0.0),
          realized_pnl_(0.0, "USD"),
          unrealized_pnl_(0.0, "USD"),
          timestamp_(timestamp) {}
    
    PositionId position_id() const { return position_id_; }
    InstrumentId instrument_id() const { return instrument_id_; }
    StrategyId strategy_id() const { return strategy_id_; }
    PositionSide side() const { return side_; }
    Quantity quantity() const { return quantity_; }
    Price entry_price() const { return entry_price_; }
    Money realized_pnl() const { return realized_pnl_; }
    Money unrealized_pnl() const { return unrealized_pnl_; }
    Timestamp timestamp() const { return timestamp_; }
    
    bool is_open() const { return side_ != PositionSide::FLAT; }
    bool is_closed() const { return side_ == PositionSide::FLAT; }
    bool is_long() const { return side_ == PositionSide::LONG; }
    bool is_short() const { return side_ == PositionSide::SHORT; }
    
    // Position updates (called by ExecutionEngine)
    void apply_fill(const Fill& fill);
    void update_unrealized_pnl(Price current_price);
    
private:
    PositionId position_id_;
    InstrumentId instrument_id_;
    StrategyId strategy_id_;
    PositionSide side_;
    Quantity quantity_;
    Price entry_price_;
    Money realized_pnl_;
    Money unrealized_pnl_;
    Timestamp timestamp_;
};

// ============================================================================
// Account
// ============================================================================

class Account {
public:
    Account() : account_id_(), balance_(0.0, "USD"), margin_used_(0.0, "USD"),
                margin_available_(0.0, "USD"), timestamp_(std::chrono::system_clock::now()) {}

    explicit Account(AccountId account_id, Timestamp timestamp = std::chrono::system_clock::now())
        : account_id_(account_id),
          balance_(0.0, "USD"),
          margin_used_(0.0, "USD"),
          margin_available_(0.0, "USD"),
          timestamp_(timestamp) {}
    
    AccountId account_id() const { return account_id_; }
    Money balance() const { return balance_; }
    Money margin_used() const { return margin_used_; }
    Money margin_available() const { return margin_available_; }
    Timestamp timestamp() const { return timestamp_; }
    
    void set_balance(Money balance) { balance_ = balance; }
    void update_margin(Money used, Money available) {
        margin_used_ = used;
        margin_available_ = available;
    }
    void set_timestamp(Timestamp ts) { timestamp_ = ts; }
    
private:
    AccountId account_id_;
    Money balance_;
    Money margin_used_;
    Money margin_available_;
    Timestamp timestamp_;
};

} // namespace npcTrading
