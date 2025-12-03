#pragma once

#include "common.hpp"
#include <vector>
#include <memory>
#include <unordered_map>

namespace npcTrading {

// Forward declarations
class Order;
class Position;
class QuoteTick;
class TradeTick;
class Bar;
class OrderBook;
class Instrument;
class Account;

// ============================================================================
// Cache Configuration
// ============================================================================

struct CacheConfig {
    size_t bar_capacity = 10000;
    size_t quote_capacity = 10000;
    size_t trade_capacity = 10000;
    size_t orderbook_depth = 10;
    bool enable_snapshot = false;
};

// ============================================================================
// Cache - Centralized state storage
// ============================================================================

/**
 * @brief Centralized in-memory storage for all system state
 * 
 * The Cache provides fast, consistent access to:
 * - Market data (quotes, trades, bars, order books)
 * - Orders (open, closed, pending)
 * - Positions (open, closed, PnL)
 * - Instruments and accounts
 * 
 * Write-before-publish guarantees temporal consistency.
 */
class Cache {
public:
    explicit Cache(const CacheConfig& config = CacheConfig());
    ~Cache();
    
    // ========================================================================
    // Order Queries
    // ========================================================================
    
    /**
     * @brief Get all open orders, optionally filtered by strategy
     */
    std::vector<Order*> orders_open(const StrategyId& strategy_id = "") const;
    
    /**
     * @brief Get all closed orders, optionally filtered by strategy
     */
    std::vector<Order*> orders_closed(const StrategyId& strategy_id = "") const;
    
    /**
     * @brief Get order by ID
     */
    Order* order(const OrderId& order_id) const;
    
    /**
     * @brief Check if order exists
     */
    bool order_exists(const OrderId& order_id) const;
    
    /**
     * @brief Add or update order
     */
    void add_order(Order* order);
    
    /**
     * @brief Update order status
     */
    void update_order(Order* order);
    
    // ========================================================================
    // Position Queries
    // ========================================================================
    
    /**
     * @brief Get all open positions, optionally filtered by strategy
     */
    std::vector<Position*> positions_open(const StrategyId& strategy_id = "") const;
    
    /**
     * @brief Get all closed positions, optionally filtered by strategy
     */
    std::vector<Position*> positions_closed(const StrategyId& strategy_id = "") const;
    
    /**
     * @brief Get position by ID
     */
    Position* position(const PositionId& position_id) const;
    
    /**
     * @brief Get position for instrument (NETTING mode)
     */
    Position* position_for_instrument(const InstrumentId& instrument_id, 
                                      const StrategyId& strategy_id = "") const;
    
    /**
     * @brief Check if position exists
     */
    bool position_exists(const PositionId& position_id) const;
    
    /**
     * @brief Add or update position
     */
    void add_position(Position* position);
    
    /**
     * @brief Update position
     */
    void update_position(Position* position);
    
    // ========================================================================
    // Market Data Queries
    // ========================================================================
    
    /**
     * @brief Get latest quote tick for instrument
     */
    QuoteTick* quote_tick(const InstrumentId& instrument_id) const;
    
    /**
     * @brief Get latest trade tick for instrument
     */
    TradeTick* trade_tick(const InstrumentId& instrument_id) const;
    
    /**
     * @brief Get latest bar for bar type
     */
    Bar* bar(const BarType& bar_type) const;
    
    /**
     * @brief Get order book for instrument
     */
    OrderBook* order_book(const InstrumentId& instrument_id) const;
    
    /**
     * @brief Add quote tick
     */
    void add_quote_tick(QuoteTick* tick);
    
    /**
     * @brief Add trade tick
     */
    void add_trade_tick(TradeTick* tick);
    
    /**
     * @brief Add bar
     */
    void add_bar(Bar* bar);
    
    /**
     * @brief Update order book
     */
    void update_order_book(OrderBook* book);
    
    // ========================================================================
    // Instrument/Account Queries
    // ========================================================================
    
    /**
     * @brief Get instrument specification
     */
    Instrument* instrument(const InstrumentId& instrument_id) const;
    
    /**
     * @brief Add instrument
     */
    void add_instrument(Instrument* instrument);
    
    /**
     * @brief Get account
     */
    Account* account(const AccountId& account_id) const;
    
    /**
     * @brief Add or update account
     */
    void add_account(Account* account);
    
    // ========================================================================
    // Cache Management
    // ========================================================================
    
    /**
     * @brief Clear all cached data
     */
    void clear();
    
    /**
     * @brief Get cache statistics
     */
    void print_stats() const;
    
private:
    CacheConfig config_;
    
    // Order storage
    std::unordered_map<OrderId, Order*> orders_;
    
    // Position storage
    std::unordered_map<PositionId, Position*> positions_;
    
    // Market data storage
    std::unordered_map<InstrumentId, QuoteTick*> quote_ticks_;
    std::unordered_map<InstrumentId, TradeTick*> trade_ticks_;
    std::unordered_map<std::string, Bar*> bars_;  // Key: bar_type string
    std::unordered_map<InstrumentId, OrderBook*> order_books_;
    
    // Instrument/Account storage
    std::unordered_map<InstrumentId, Instrument*> instruments_;
    std::unordered_map<AccountId, Account*> accounts_;
    
    // TODO: Add indexing for fast queries
    // TODO: Add LRU eviction for capacity management
    // TODO: Add snapshot/restore functionality
};

} // namespace npcTrading
