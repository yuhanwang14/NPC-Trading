#pragma once

#include "common.hpp"
#include "message_bus.hpp"
#include <string>
#include <utility>

namespace npcTrading {

class Message;
class MarketDataMessage;

// ============================================================================
// Instrument Specification
// ============================================================================

class Instrument {
public:
    Instrument() = default;
    Instrument(InstrumentId id,
               const std::string& symbol,
               VenueId venue,
               double tick_size,
               double step_size,
               double min_quantity,
               double max_quantity,
               Timestamp timestamp = std::chrono::system_clock::now())
        : id_(id), symbol_(symbol), venue_(venue),
          tick_size_(tick_size), step_size_(step_size),
          min_quantity_(min_quantity), max_quantity_(max_quantity),
          timestamp_(timestamp) {}
    
    InstrumentId id() const { return id_; }
    std::string symbol() const { return symbol_; }
    VenueId venue() const { return venue_; }
    double tick_size() const { return tick_size_; }
    double step_size() const { return step_size_; }
    double min_quantity() const { return min_quantity_; }
    double max_quantity() const { return max_quantity_; }
    Timestamp timestamp() const { return timestamp_; }
    
private:
    InstrumentId id_;
    std::string symbol_;
    VenueId venue_;
    double tick_size_;     // Minimum price increment
    double step_size_;     // Minimum quantity increment
    double min_quantity_;  // Minimum order size
    double max_quantity_;  // Maximum order size
    Timestamp timestamp_;
};

// ============================================================================
// Market Data Types
// ============================================================================

/// Quote tick (best bid/ask)
class QuoteTick {
public:
    QuoteTick() = default;
    QuoteTick(InstrumentId instrument_id,
              Price bid_price,
              Price ask_price,
              Quantity bid_size,
              Quantity ask_size,
              Timestamp timestamp)
        : instrument_id_(instrument_id),
          bid_price_(bid_price),
          ask_price_(ask_price),
          bid_size_(bid_size),
          ask_size_(ask_size),
          timestamp_(timestamp) {}
    
    InstrumentId instrument_id() const { return instrument_id_; }
    Price bid_price() const { return bid_price_; }
    Price ask_price() const { return ask_price_; }
    Quantity bid_size() const { return bid_size_; }
    Quantity ask_size() const { return ask_size_; }
    Timestamp timestamp() const { return timestamp_; }
    
private:
    InstrumentId instrument_id_;
    Price bid_price_;
    Price ask_price_;
    Quantity bid_size_;
    Quantity ask_size_;
    Timestamp timestamp_;
};

/// Trade tick (executed trade)
class TradeTick {
public:
    TradeTick() = default;
    TradeTick(InstrumentId instrument_id,
              Price price,
              Quantity size,
              OrderSide aggressor_side,
              Timestamp timestamp)
        : instrument_id_(instrument_id),
          price_(price),
          size_(size),
          aggressor_side_(aggressor_side),
          timestamp_(timestamp) {}
    
    InstrumentId instrument_id() const { return instrument_id_; }
    Price price() const { return price_; }
    Quantity size() const { return size_; }
    OrderSide aggressor_side() const { return aggressor_side_; }
    Timestamp timestamp() const { return timestamp_; }
    
private:
    InstrumentId instrument_id_;
    Price price_;
    Quantity size_;
    OrderSide aggressor_side_;
    Timestamp timestamp_;
};

/// Bar (OHLCV candle)
class Bar {
public:
    Bar() = default;
    Bar(BarType bar_type,
        Price open,
        Price high,
        Price low,
        Price close,
        Quantity volume,
        Timestamp timestamp)
        : bar_type_(bar_type),
          open_(open),
          high_(high),
          low_(low),
          close_(close),
          volume_(volume),
          timestamp_(timestamp) {}
    
    BarType bar_type() const { return bar_type_; }
    Price open() const { return open_; }
    Price high() const { return high_; }
    Price low() const { return low_; }
    Price close() const { return close_; }
    Quantity volume() const { return volume_; }
    Timestamp timestamp() const { return timestamp_; }
    
private:
    BarType bar_type_;
    Price open_;
    Price high_;
    Price low_;
    Price close_;
    Quantity volume_;
    Timestamp timestamp_;
};

/// Order book level
struct OrderBookLevel {
    Price price;
    Quantity size;
    int order_count;
    
    OrderBookLevel() : price(0), size(0), order_count(0) {}
    OrderBookLevel(Price p, Quantity s, int c = 0) 
        : price(p), size(s), order_count(c) {}
};

/// Order book depth enum
enum class OrderBookDepth {
    L1,    // Best bid/ask only (top of book)
    L2,    // Price-aggregated depth (multiple levels)
    L3     // Full order-level depth (rare, exchange-specific)
};

/// Order book delta/update type
enum class OrderBookDeltaType {
    ADD,      // New level added
    UPDATE,   // Existing level quantity changed
    DELETE,   // Level removed (quantity = 0)
    SNAPSHOT  // Full order book snapshot
};

/// Order book delta (incremental update)
struct OrderBookDelta {
    OrderBookDeltaType type;
    OrderSide side;
    Price price;
    Quantity size;
    Timestamp timestamp;
    
    OrderBookDelta(OrderBookDeltaType t, OrderSide s, Price p, Quantity sz, Timestamp ts)
        : type(t), side(s), price(p), size(sz), timestamp(ts) {}
};

/// Order book statistics
struct OrderBookStats {
    double spread;              // Best ask - best bid
    double mid_price;           // (Best bid + best ask) / 2
    double weighted_mid_price;  // Volume-weighted mid price
    double imbalance;           // (bid_volume - ask_volume) / (bid_volume + ask_volume)
    Quantity total_bid_volume;
    Quantity total_ask_volume;
    int bid_levels;
    int ask_levels;
    
    OrderBookStats() : spread(0), mid_price(0), weighted_mid_price(0), 
                      imbalance(0), total_bid_volume(0), total_ask_volume(0),
                      bid_levels(0), ask_levels(0) {}
};

/// Order book (depth snapshot with enhanced functionality)
class OrderBook {
public:
    OrderBook() : depth_level_(OrderBookDepth::L2), sequence_(0) {}
    OrderBook(InstrumentId instrument_id,
              const std::vector<OrderBookLevel>& bids,
              const std::vector<OrderBookLevel>& asks,
              Timestamp timestamp,
              OrderBookDepth depth = OrderBookDepth::L2,
              uint64_t sequence = 0)
        : instrument_id_(instrument_id),
          bids_(bids),
          asks_(asks),
          timestamp_(timestamp),
          depth_level_(depth),
          sequence_(sequence) {
        compute_stats();
    }
    
    // ========================================================================
    // Basic Accessors
    // ========================================================================
    
    InstrumentId instrument_id() const { return instrument_id_; }
    const std::vector<OrderBookLevel>& bids() const { return bids_; }
    const std::vector<OrderBookLevel>& asks() const { return asks_; }
    Timestamp timestamp() const { return timestamp_; }
    OrderBookDepth depth_level() const { return depth_level_; }
    uint64_t sequence() const { return sequence_; }
    
    // ========================================================================
    // Top of Book (L1)
    // ========================================================================
    
    Price best_bid_price() const { return bids_.empty() ? Price(0) : bids_[0].price; }
    Price best_ask_price() const { return asks_.empty() ? Price(0) : asks_[0].price; }
    Quantity best_bid_size() const { return bids_.empty() ? Quantity(0) : bids_[0].size; }
    Quantity best_ask_size() const { return asks_.empty() ? Quantity(0) : asks_[0].size; }
    
    // ========================================================================
    // Incremental Updates
    // ========================================================================
    
    /**
     * @brief Apply incremental delta update to order book
     * @param delta The delta to apply
     * @return true if successful, false if delta is stale or invalid
     */
    bool apply_delta(const OrderBookDelta& delta);
    
    /**
     * @brief Clear all levels and reset to empty state
     */
    void clear();
    
    // ========================================================================
    // Analytics and Statistics
    // ========================================================================
    
    const OrderBookStats& stats() const { return stats_; }
    
    double spread() const { return stats_.spread; }
    double mid_price() const { return stats_.mid_price; }
    double imbalance() const { return stats_.imbalance; }
    
    /**
     * @brief Calculate volume-weighted average price (VWAP) for given quantity
     * @param side BUY (consume asks) or SELL (consume bids)
     * @param quantity Quantity to simulate
     * @return VWAP or 0 if insufficient liquidity
     */
    double calculate_vwap(OrderSide side, Quantity quantity) const;
    
    /**
     * @brief Calculate market impact for a given quantity
     * @param side BUY or SELL
     * @param quantity Order quantity
     * @return Price impact as percentage (e.g., 0.05 = 5%)
     */
    double calculate_market_impact(OrderSide side, Quantity quantity) const;
    
    /**
     * @brief Get cumulative volume at price level
     * @param side BID or ASK
     * @param price_level Price threshold
     * @return Total volume up to that level
     */
    Quantity get_volume_at_level(OrderSide side, Price price_level) const;
    
    /**
     * @brief Check if order book is crossed (bid >= ask) - indicates invalid state
     */
    bool is_crossed() const;
    
    /**
     * @brief Check if order book is locked (bid == ask)
     */
    bool is_locked() const;
    
    /**
     * @brief Validate order book integrity (sorted, no duplicates, etc.)
     */
    bool validate() const;
    
    // ========================================================================
    // Modifiers
    // ========================================================================
    
    void set_sequence(uint64_t seq) { sequence_ = seq; }
    void set_timestamp(Timestamp ts) { timestamp_ = ts; }
    
private:
    InstrumentId instrument_id_;
    std::vector<OrderBookLevel> bids_;  // Sorted descending (highest bid first)
    std::vector<OrderBookLevel> asks_;  // Sorted ascending (lowest ask first)
    Timestamp timestamp_;
    OrderBookDepth depth_level_;
    uint64_t sequence_;  // Monotonic sequence number for tracking updates
    
    OrderBookStats stats_;
    
    /// Recompute statistics after updates
    void compute_stats();
    
    /// Insert level maintaining sort order
    void insert_level(std::vector<OrderBookLevel>& levels, const OrderBookLevel& level, bool descending);
    
    /// Remove level by price
    void remove_level(std::vector<OrderBookLevel>& levels, Price price);
    
    /// Update level quantity by price
    bool update_level(std::vector<OrderBookLevel>& levels, Price price, Quantity new_size);
};

// ============================================================================
// Market Data Messages (for MessageBus transport)
// ============================================================================

class MarketDataMessage : public Message {
public:
    virtual void dispatch_to(class Actor& actor) const = 0;
};

class QuoteTickMessage : public MarketDataMessage {
public:
    explicit QuoteTickMessage(QuoteTick tick) : tick_(std::move(tick)) {}

    Timestamp timestamp() const override { return tick_.timestamp(); }
    std::string type() const override { return "QuoteTick"; }
    const QuoteTick& tick() const { return tick_; }
    void dispatch_to(class Actor& actor) const override;

private:
    QuoteTick tick_;
};

class TradeTickMessage : public MarketDataMessage {
public:
    explicit TradeTickMessage(TradeTick tick) : tick_(std::move(tick)) {}

    Timestamp timestamp() const override { return tick_.timestamp(); }
    std::string type() const override { return "TradeTick"; }
    const TradeTick& trade() const { return tick_; }
    void dispatch_to(class Actor& actor) const override;

private:
    TradeTick tick_;
};

class BarMessage : public MarketDataMessage {
public:
    explicit BarMessage(Bar bar) : bar_(std::move(bar)) {}

    Timestamp timestamp() const override { return bar_.timestamp(); }
    std::string type() const override { return "Bar"; }
    const Bar& bar() const { return bar_; }
    void dispatch_to(class Actor& actor) const override;

private:
    Bar bar_;
};

class OrderBookMessage : public MarketDataMessage {
public:
    explicit OrderBookMessage(OrderBook book) : book_(std::move(book)) {}

    Timestamp timestamp() const override { return book_.timestamp(); }
    std::string type() const override { return "OrderBook"; }
    const OrderBook& book() const { return book_; }
    void dispatch_to(class Actor& actor) const override;

private:
    OrderBook book_;
};

} // namespace npcTrading
