#pragma once

#include "common.hpp"
#include "model.hpp"
#include "market_data.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>

namespace npcTrading {

using BookFrequency = std::string;

// ============================================================================
// Simple fixed-capacity ring buffer (overwrites oldest on overflow)
// ============================================================================
template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity = 0)
        : buffer_(capacity), head_(0), size_(0) {}

    void push(const T& value) {
        if (buffer_.empty()) {
            return;
        }
        buffer_[head_] = value;
        head_ = (head_ + 1) % buffer_.size();
        if (size_ < buffer_.size()) {
            ++size_;
        }
    }

    std::vector<T> recent(size_t n) const {
        n = std::min(n, size_);
        std::vector<T> out;
        out.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            size_t idx = (head_ + buffer_.size() - 1 - i) % buffer_.size();
            out.push_back(buffer_[idx]);
        }
        std::reverse(out.begin(), out.end()); // return in chronological order
        return out;
    }

    size_t size() const { return size_; }
    size_t capacity() const { return buffer_.size(); }
    bool empty() const { return size_ == 0; }

private:
    std::vector<T> buffer_;
    size_t head_;
    size_t size_;
};

// ============================================================================
// Cache Configuration
// ============================================================================

struct CacheConfig {
    size_t trade_capacity = 10000;     // trade history depth
    size_t bar_capacity = 0;           // bar history depth
    size_t orderbook_capacity = 0;     // order book history depth per frequency
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
    std::vector<const Order*> orders_open(const StrategyId& strategy_id = "") const;
    
    /**
     * @brief Get all closed orders, optionally filtered by strategy
     */
    std::vector<const Order*> orders_closed(const StrategyId& strategy_id = "") const;
    
    /**
     * @brief Get order by ID
     */
    const Order* order(const OrderId& order_id) const;
    
    /**
     * @brief Check if order exists
     */
    bool order_exists(const OrderId& order_id) const;
    
    /**
     * @brief Add or update order
     */
    void add_order(Order order);
    
    /**
     * @brief Update order status
     */
    void update_order(const Order& order);
    
    // ========================================================================
    // Position Queries
    // ========================================================================
    
    /**
     * @brief Get all open positions, optionally filtered by strategy
     */
    std::vector<const Position*> positions_open(const StrategyId& strategy_id = "") const;
    
    /**
     * @brief Get all closed positions, optionally filtered by strategy
     */
    std::vector<const Position*> positions_closed(const StrategyId& strategy_id = "") const;
    
    /**
     * @brief Get position by ID
     */
    const Position* position(const PositionId& position_id) const;
    
    /**
     * @brief Get position for instrument (NETTING mode)
     */
    const Position* position_for_instrument(const InstrumentId& instrument_id, 
                                            const StrategyId& strategy_id = "") const;
    
    /**
     * @brief Check if position exists
     */
    bool position_exists(const PositionId& position_id) const;
    
    /**
     * @brief Add or update position
     */
    void add_position(Position position);
    
    /**
     * @brief Update position
     */
    void update_position(const Position& position);
    
    // ========================================================================
    // Market Data Queries
    // ========================================================================
    
    /**
     * @brief Get latest quote tick for instrument
     */
    const QuoteTick* quote_tick(const InstrumentId& instrument_id) const;
    
    /**
     * @brief Get latest trade tick for instrument
     */
    const TradeTick* trade_tick(const InstrumentId& instrument_id) const;
    
    /**
     * @brief Get latest bar for bar type
     */
    const Bar* bar(const BarType& bar_type) const;

    /**
     * @brief Get recent quotes for instrument (up to n, chronological). Quote history is not stored; returns latest only.
     */
    std::vector<QuoteTick> recent_quotes(const InstrumentId& instrument_id, size_t n) const;

    /**
     * @brief Get recent trades for instrument (up to n, chronological)
     */
    std::vector<TradeTick> recent_trades(const InstrumentId& instrument_id, size_t n) const;

    /**
     * @brief Get recent bars for bar type (up to n, chronological)
     */
    std::vector<Bar> recent_bars(const BarType& bar_type, size_t n) const;
    
    /**
     * @brief Get order book for instrument and frequency
     */
    const OrderBook* order_book(const InstrumentId& instrument_id,
                                const BookFrequency& frequency = "default") const;
    /**
     * @brief Get recent order book snapshots (up to n, chronological)
     */
    std::vector<OrderBook> recent_order_books(const InstrumentId& instrument_id,
                                              const BookFrequency& frequency,
                                              size_t n) const;
    
    /**
     * @brief Add quote tick
     */
    void add_quote_tick(const QuoteTick& tick);
    
    /**
     * @brief Add trade tick
     */
    void add_trade_tick(const TradeTick& tick);
    
    /**
     * @brief Add bar
     */
    void add_bar(const Bar& bar);
    
    /**
     * @brief Update order book
     */
    void update_order_book(const OrderBook& book, const BookFrequency& frequency = "default");
    
    // ========================================================================
    // Instrument/Account Queries
    // ========================================================================
    
    /**
     * @brief Get instrument specification
     */
    const Instrument* instrument(const InstrumentId& instrument_id) const;
    
    /**
     * @brief Add instrument
     */
    void add_instrument(const Instrument& instrument);
    
    /**
     * @brief Get account
     */
    const Account* account(const AccountId& account_id) const;
    
    /**
     * @brief Add or update account
     */
    void add_account(const Account& account);
    
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
    // Helper to build bar key (instrument|spec) - matches DataEngine::bar_key
    static std::string bar_key(const BarType& bar_type) {
        return bar_type.instrument_id() + "|" + bar_type.spec();
    }
    
    CacheConfig config_;
    
    // Order storage
    std::unordered_map<OrderId, std::unique_ptr<Order>> orders_;
    
    // Position storage
    std::unordered_map<PositionId, std::unique_ptr<Position>> positions_;
    
    // Market data storage
    std::unordered_map<InstrumentId, QuoteTick> quote_ticks_;
    std::unordered_map<InstrumentId, TradeTick> trade_ticks_;
    std::unordered_map<std::string, Bar> bars_;  // Key: bar_type string
    std::unordered_map<std::string, OrderBook> order_books_; // Key: instrument|frequency
    std::unordered_map<InstrumentId, RingBuffer<TradeTick>> trade_history_;
    std::unordered_map<std::string, RingBuffer<Bar>> bar_history_;
    std::unordered_map<std::string, RingBuffer<OrderBook>> order_book_history_;
    
    // Latest timestamps to filter out stale data
    std::unordered_map<InstrumentId, Timestamp> latest_quote_ts_;
    std::unordered_map<InstrumentId, Timestamp> latest_trade_ts_;
    std::unordered_map<std::string, Timestamp> latest_bar_ts_;
    std::unordered_map<std::string, Timestamp> latest_order_book_ts_;
    std::unordered_map<OrderId, Timestamp> latest_order_ts_;
    std::unordered_map<PositionId, Timestamp> latest_position_ts_;
    std::unordered_map<InstrumentId, Timestamp> latest_instrument_ts_;
    std::unordered_map<AccountId, Timestamp> latest_account_ts_;
    
    // Instrument/Account storage
    std::unordered_map<InstrumentId, Instrument> instruments_;
    std::unordered_map<AccountId, Account> accounts_;
    
    // TODO: Add indexing for fast queries
    // TODO: Add LRU eviction for capacity management
    // TODO: Add snapshot/restore functionality
};

} // namespace npcTrading
