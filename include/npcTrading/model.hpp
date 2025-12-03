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
          TimeInForce time_in_force = TimeInForce::GTC)
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
          status_(OrderStatus::INITIALIZED) {}
    
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
    
    // Status checks
    bool is_open() const;
    bool is_closed() const;
    bool is_filled() const { return status_ == OrderStatus::FILLED; }
    
    // Modifications
    void set_status(OrderStatus status) { status_ = status; }
    void set_filled_qty(Quantity qty) { filled_qty_ = qty; }
    void set_position_id(const PositionId& position_id) { position_id_ = position_id; }
    void update_price(Price new_price) { price_ = new_price; }
    void update_quantity(Quantity new_quantity) { quantity_ = new_quantity; }
    
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
             StrategyId strategy_id)
        : position_id_(position_id),
          instrument_id_(instrument_id),
          strategy_id_(strategy_id),
          side_(PositionSide::FLAT),
          quantity_(0.0),
          entry_price_(0.0),
          realized_pnl_(0.0, "USD"),
          unrealized_pnl_(0.0, "USD") {}
    
    PositionId position_id() const { return position_id_; }
    InstrumentId instrument_id() const { return instrument_id_; }
    StrategyId strategy_id() const { return strategy_id_; }
    PositionSide side() const { return side_; }
    Quantity quantity() const { return quantity_; }
    Price entry_price() const { return entry_price_; }
    Money realized_pnl() const { return realized_pnl_; }
    Money unrealized_pnl() const { return unrealized_pnl_; }
    
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
};

// ============================================================================
// Account
// ============================================================================

class Account {
public:
    explicit Account(AccountId account_id)
        : account_id_(account_id),
          balance_(0.0, "USD"),
          margin_used_(0.0, "USD"),
          margin_available_(0.0, "USD") {}
    
    AccountId account_id() const { return account_id_; }
    Money balance() const { return balance_; }
    Money margin_used() const { return margin_used_; }
    Money margin_available() const { return margin_available_; }
    
    void set_balance(Money balance) { balance_ = balance; }
    void update_margin(Money used, Money available) {
        margin_used_ = used;
        margin_available_ = available;
    }
    
private:
    AccountId account_id_;
    Money balance_;
    Money margin_used_;
    Money margin_available_;
};

} // namespace npcTrading
