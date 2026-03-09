#include "npcTrading/market_data.hpp"
#include "npcTrading/strategy.hpp"
#include <algorithm>
#include <cmath>

namespace npcTrading {

void QuoteTickMessage::dispatch_to(Actor& actor) const {
    actor.on_quote(tick_);
}

void TradeTickMessage::dispatch_to(Actor& actor) const {
    actor.on_trade(tick_);
}

void BarMessage::dispatch_to(Actor& actor) const {
    actor.on_bar(bar_);
}

void OrderBookMessage::dispatch_to(Actor& actor) const {
    actor.on_order_book(book_);
}

// ============================================================================
// OrderBook Enhanced Implementation
// ============================================================================

void OrderBook::compute_stats() {
    stats_ = OrderBookStats();
    
    if (bids_.empty() || asks_.empty()) {
        return;
    }
    
    // Top of book
    double best_bid = bids_[0].price.as_double();
    double best_ask = asks_[0].price.as_double();
    
    stats_.spread = best_ask - best_bid;
    stats_.mid_price = (best_bid + best_ask) / 2.0;
    
    // Calculate total volumes
    double total_bid_qty = 0.0;
    double total_ask_qty = 0.0;
    double weighted_bid_sum = 0.0;
    double weighted_ask_sum = 0.0;
    
    for (const auto& level : bids_) {
        double qty = level.size.as_double();
        total_bid_qty += qty;
        weighted_bid_sum += level.price.as_double() * qty;
    }
    
    for (const auto& level : asks_) {
        double qty = level.size.as_double();
        total_ask_qty += qty;
        weighted_ask_sum += level.price.as_double() * qty;
    }
    
    stats_.total_bid_volume = Quantity(total_bid_qty);
    stats_.total_ask_volume = Quantity(total_ask_qty);
    stats_.bid_levels = static_cast<int>(bids_.size());
    stats_.ask_levels = static_cast<int>(asks_.size());
    
    // Imbalance: positive means more buying pressure
    double total_volume = total_bid_qty + total_ask_qty;
    if (total_volume > 0) {
        stats_.imbalance = (total_bid_qty - total_ask_qty) / total_volume;
    }
    
    // Weighted mid price
    if (total_bid_qty > 0 && total_ask_qty > 0) {
        double weighted_bid = weighted_bid_sum / total_bid_qty;
        double weighted_ask = weighted_ask_sum / total_ask_qty;
        stats_.weighted_mid_price = (weighted_bid + weighted_ask) / 2.0;
    } else {
        stats_.weighted_mid_price = stats_.mid_price;
    }
}

bool OrderBook::apply_delta(const OrderBookDelta& delta) {
    auto& levels = (delta.side == OrderSide::BUY) ? bids_ : asks_;
    bool is_descending = (delta.side == OrderSide::BUY);
    
    switch (delta.type) {
        case OrderBookDeltaType::ADD:
        case OrderBookDeltaType::UPDATE: {
            // Remove existing level if present
            remove_level(levels, delta.price);
            // Add new level if size > 0
            if (delta.size.as_double() > 0) {
                insert_level(levels, OrderBookLevel(delta.price, delta.size), is_descending);
            }
            break;
        }
        case OrderBookDeltaType::DELETE: {
            remove_level(levels, delta.price);
            break;
        }
        case OrderBookDeltaType::SNAPSHOT: {
            // For snapshot, this shouldn't be called - use constructor instead
            return false;
        }
    }
    
    timestamp_ = delta.timestamp;
    compute_stats();
    return true;
}

void OrderBook::clear() {
    bids_.clear();
    asks_.clear();
    sequence_ = 0;
    stats_ = OrderBookStats();
}

double OrderBook::calculate_vwap(OrderSide side, Quantity quantity) const {
    const auto& levels = (side == OrderSide::BUY) ? asks_ : bids_;
    
    double remaining = quantity.as_double();
    double total_cost = 0.0;
    double total_qty = 0.0;
    
    for (const auto& level : levels) {
        double level_qty = level.size.as_double();
        double consume_qty = std::min(remaining, level_qty);
        
        total_cost += level.price.as_double() * consume_qty;
        total_qty += consume_qty;
        remaining -= consume_qty;
        
        if (remaining <= 0) break;
    }
    
    if (total_qty > 0) {
        return total_cost / total_qty;
    }
    return 0.0;
}

double OrderBook::calculate_market_impact(OrderSide side, Quantity quantity) const {
    double vwap = calculate_vwap(side, quantity);
    if (vwap == 0.0) return 0.0;
    
    double reference_price = mid_price();
    if (reference_price == 0.0) return 0.0;
    
    // Impact as percentage difference from mid price
    return (vwap - reference_price) / reference_price;
}

Quantity OrderBook::get_volume_at_level(OrderSide side, Price price_level) const {
    const auto& levels = (side == OrderSide::BUY) ? bids_ : asks_;
    
    double cumulative = 0.0;
    double price_threshold = price_level.as_double();
    
    for (const auto& level : levels) {
        double level_price = level.price.as_double();
        
        // For bids: accumulate levels >= price_level
        // For asks: accumulate levels <= price_level
        bool include = (side == OrderSide::BUY) ? 
                      (level_price >= price_threshold) : 
                      (level_price <= price_threshold);
        
        if (include) {
            cumulative += level.size.as_double();
        }
    }
    
    return Quantity(cumulative);
}

bool OrderBook::is_crossed() const {
    if (bids_.empty() || asks_.empty()) return false;
    return bids_[0].price.as_double() >= asks_[0].price.as_double();
}

bool OrderBook::is_locked() const {
    if (bids_.empty() || asks_.empty()) return false;
    return std::abs(bids_[0].price.as_double() - asks_[0].price.as_double()) < 1e-8;
}

bool OrderBook::validate() const {
    // Check bids are sorted descending
    for (size_t i = 1; i < bids_.size(); ++i) {
        if (bids_[i].price.as_double() > bids_[i-1].price.as_double()) {
            return false;
        }
    }
    
    // Check asks are sorted ascending
    for (size_t i = 1; i < asks_.size(); ++i) {
        if (asks_[i].price.as_double() < asks_[i-1].price.as_double()) {
            return false;
        }
    }
    
    // Check not crossed
    if (is_crossed()) {
        return false;
    }
    
    return true;
}

void OrderBook::insert_level(std::vector<OrderBookLevel>& levels, 
                             const OrderBookLevel& level, 
                             bool descending) {
    auto it = levels.begin();
    
    if (descending) {
        // For bids: insert in descending order (highest price first)
        while (it != levels.end() && it->price.as_double() > level.price.as_double()) {
            ++it;
        }
    } else {
        // For asks: insert in ascending order (lowest price first)
        while (it != levels.end() && it->price.as_double() < level.price.as_double()) {
            ++it;
        }
    }
    
    levels.insert(it, level);
}

void OrderBook::remove_level(std::vector<OrderBookLevel>& levels, Price price) {
    auto it = std::remove_if(levels.begin(), levels.end(), 
        [&price](const OrderBookLevel& level) {
            return std::abs(level.price.as_double() - price.as_double()) < 1e-8;
        });
    levels.erase(it, levels.end());
}

bool OrderBook::update_level(std::vector<OrderBookLevel>& levels, 
                             Price price, 
                             Quantity new_size) {
    for (auto& level : levels) {
        if (std::abs(level.price.as_double() - price.as_double()) < 1e-8) {
            level.size = new_size;
            return true;
        }
    }
    return false;
}

} // namespace npcTrading
