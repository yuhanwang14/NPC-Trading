#include "npcTrading/cache.hpp"
#include "npcTrading/model.hpp"
#include "npcTrading/market_data.hpp"

namespace npcTrading {

Cache::Cache(const CacheConfig& config) : config_(config) {
}

Cache::~Cache() {
    clear();
}

// Order operations
std::vector<Order*> Cache::orders_open(const StrategyId& strategy_id) const {
    // TODO: Implement filtering by strategy_id
    std::vector<Order*> result;
    for (const auto& pair : orders_) {
        if (pair.second->is_open()) {
            if (strategy_id.empty() || pair.second->strategy_id() == strategy_id) {
                result.push_back(pair.second);
            }
        }
    }
    return result;
}

std::vector<Order*> Cache::orders_closed(const StrategyId& strategy_id) const {
    std::vector<Order*> result;
    for (const auto& pair : orders_) {
        if (pair.second->is_closed()) {
            if (strategy_id.empty() || pair.second->strategy_id() == strategy_id) {
                result.push_back(pair.second);
            }
        }
    }
    return result;
}

Order* Cache::order(const OrderId& order_id) const {
    auto it = orders_.find(order_id);
    return (it != orders_.end()) ? it->second : nullptr;
}

bool Cache::order_exists(const OrderId& order_id) const {
    return orders_.find(order_id) != orders_.end();
}

void Cache::add_order(Order* order) {
    orders_[order->order_id()] = order;
}

void Cache::update_order(Order* order) {
    // Order is updated in-place, just verify it exists
    if (!order_exists(order->order_id())) {
        add_order(order);
    }
}

// Position operations
std::vector<Position*> Cache::positions_open(const StrategyId& strategy_id) const {
    std::vector<Position*> result;
    for (const auto& pair : positions_) {
        if (pair.second->is_open()) {
            if (strategy_id.empty() || pair.second->strategy_id() == strategy_id) {
                result.push_back(pair.second);
            }
        }
    }
    return result;
}

std::vector<Position*> Cache::positions_closed(const StrategyId& strategy_id) const {
    std::vector<Position*> result;
    for (const auto& pair : positions_) {
        if (pair.second->is_closed()) {
            if (strategy_id.empty() || pair.second->strategy_id() == strategy_id) {
                result.push_back(pair.second);
            }
        }
    }
    return result;
}

Position* Cache::position(const PositionId& position_id) const {
    auto it = positions_.find(position_id);
    return (it != positions_.end()) ? it->second : nullptr;
}

Position* Cache::position_for_instrument(const InstrumentId& instrument_id,
                                         const StrategyId& strategy_id) const {
    // TODO: Implement proper lookup
    for (const auto& pair : positions_) {
        if (pair.second->instrument_id() == instrument_id &&
            pair.second->is_open()) {
            if (strategy_id.empty() || pair.second->strategy_id() == strategy_id) {
                return pair.second;
            }
        }
    }
    return nullptr;
}

bool Cache::position_exists(const PositionId& position_id) const {
    return positions_.find(position_id) != positions_.end();
}

void Cache::add_position(Position* position) {
    positions_[position->position_id()] = position;
}

void Cache::update_position(Position* position) {
    if (!position_exists(position->position_id())) {
        add_position(position);
    }
}

// Market data operations
QuoteTick* Cache::quote_tick(const InstrumentId& instrument_id) const {
    auto it = quote_ticks_.find(instrument_id);
    return (it != quote_ticks_.end()) ? it->second : nullptr;
}

TradeTick* Cache::trade_tick(const InstrumentId& instrument_id) const {
    auto it = trade_ticks_.find(instrument_id);
    return (it != trade_ticks_.end()) ? it->second : nullptr;
}

Bar* Cache::bar(const BarType& bar_type) const {
    auto it = bars_.find(bar_type.spec());
    return (it != bars_.end()) ? it->second : nullptr;
}

OrderBook* Cache::order_book(const InstrumentId& instrument_id) const {
    auto it = order_books_.find(instrument_id);
    return (it != order_books_.end()) ? it->second : nullptr;
}

void Cache::add_quote_tick(QuoteTick* tick) {
    quote_ticks_[tick->instrument_id()] = tick;
}

void Cache::add_trade_tick(TradeTick* tick) {
    trade_ticks_[tick->instrument_id()] = tick;
}

void Cache::add_bar(Bar* bar) {
    bars_[bar->bar_type().spec()] = bar;
}

void Cache::update_order_book(OrderBook* book) {
    order_books_[book->instrument_id()] = book;
}

// Instrument/Account operations
Instrument* Cache::instrument(const InstrumentId& instrument_id) const {
    auto it = instruments_.find(instrument_id);
    return (it != instruments_.end()) ? it->second : nullptr;
}

void Cache::add_instrument(Instrument* instrument) {
    instruments_[instrument->id()] = instrument;
}

Account* Cache::account(const AccountId& account_id) const {
    auto it = accounts_.find(account_id);
    return (it != accounts_.end()) ? it->second : nullptr;
}

void Cache::add_account(Account* account) {
    accounts_[account->account_id()] = account;
}

void Cache::clear() {
    // TODO: Proper cleanup with memory management
    orders_.clear();
    positions_.clear();
    quote_ticks_.clear();
    trade_ticks_.clear();
    bars_.clear();
    order_books_.clear();
    instruments_.clear();
    accounts_.clear();
}

void Cache::print_stats() const {
    // TODO: Implement statistics
}

} // namespace npcTrading
