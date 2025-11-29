#pragma once

#include <string>
#include <cstdint>
#include <chrono>
#include <memory>

namespace npcTrading {

// ============================================================================
// Core Type Definitions
// ============================================================================

/// Unique identifier types
using ComponentId = std::string;
using StrategyId = std::string;
using OrderId = std::string;
using PositionId = std::string;
using InstrumentId = std::string;
using AccountId = std::string;
using ClientId = std::string;
using VenueId = std::string;

/// Timestamp type
using Timestamp = std::chrono::system_clock::time_point;
using TimestampNs = int64_t; // Nanoseconds since epoch

// ============================================================================
// Enumerations
// ============================================================================

/// Component state in lifecycle
enum class ComponentState {
    PRE_INITIALIZED,
    INITIALIZED,
    READY,
    RUNNING,
    STOPPED,
    DISPOSED
};

/// Order side
enum class OrderSide {
    BUY,
    SELL
};

/// Order type
enum class OrderType {
    MARKET,
    LIMIT,
    STOP_MARKET,
    STOP_LIMIT
};

/// Time in force
enum class TimeInForce {
    GTC,  // Good Till Cancel
    IOC,  // Immediate Or Cancel
    FOK,  // Fill Or Kill
    GTD   // Good Till Date
};

/// Order status
enum class OrderStatus {
    INITIALIZED,
    SUBMITTED,
    ACCEPTED,
    REJECTED,
    CANCELED,
    EXPIRED,
    TRIGGERED,
    PARTIALLY_FILLED,
    FILLED
};

/// Position side
enum class PositionSide {
    LONG,
    SHORT,
    FLAT
};

/// OMS (Order Management System) type
enum class OmsType {
    NETTING,   // Single position per instrument
    HEDGING    // Multiple positions allowed
};

/// Trading state
enum class TradingState {
    ACTIVE,    // Accept all valid orders
    HALTED,    // Reject all new orders
    REDUCING   // Only accept orders that reduce positions
};

// ============================================================================
// Value Types
// ============================================================================

/// Price representation
class Price {
public:
    Price() : value_(0.0) {}
    explicit Price(double value) : value_(value) {}
    
    double as_double() const { return value_; }
    
    // TODO: Add arithmetic operators and precision handling
    
private:
    double value_;
};

/// Quantity representation
class Quantity {
public:
    Quantity() : value_(0.0) {}
    explicit Quantity(double value) : value_(value) {}
    
    double as_double() const { return value_; }
    
    // TODO: Add arithmetic operators and precision handling
    
private:
    double value_;
};

/// Money representation
class Money {
public:
    Money() : value_(0.0), currency_("USD") {}
    Money(double value, const std::string& currency) 
        : value_(value), currency_(currency) {}
    
    double as_double() const { return value_; }
    std::string currency() const { return currency_; }
    
    // TODO: Add arithmetic operators
    
private:
    double value_;
    std::string currency_;
};

// ============================================================================
// Bar Type
// ============================================================================

/// Bar aggregation specification
class BarType {
public:
    BarType(const InstrumentId& instrument_id, const std::string& spec)
        : instrument_id_(instrument_id), spec_(spec) {}
    
    InstrumentId instrument_id() const { return instrument_id_; }
    std::string spec() const { return spec_; }
    
    // TODO: Parse spec (e.g., "1-MINUTE", "5-MINUTE", "1-HOUR")
    
private:
    InstrumentId instrument_id_;
    std::string spec_;
};

// ============================================================================
// String conversion utilities
// ============================================================================

std::string to_string(ComponentState state);
std::string to_string(OrderSide side);
std::string to_string(OrderType type);
std::string to_string(TimeInForce tif);
std::string to_string(OrderStatus status);
std::string to_string(TradingState state);

} // namespace npcTrading
