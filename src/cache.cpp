#include "npcTrading/cache.hpp"
#include "npcTrading/model.hpp"
#include "npcTrading/market_data.hpp"
#include <algorithm>

namespace npcTrading {

namespace {
template <typename Key>
bool is_stale_and_update(std::unordered_map<Key, Timestamp>& latest_map,
                         const Key& key,
                         Timestamp incoming_ts) {
    auto it = latest_map.find(key);
    if (it != latest_map.end() && incoming_ts <= it->second) {
        return true; // incoming data is older or same, drop
    }
    latest_map[key] = incoming_ts;
    return false;
}

std::string order_book_key(const InstrumentId& instrument_id, const BookFrequency& frequency) {
    return instrument_id + "|" + frequency;
}
} // namespace

Cache::Cache(const CacheConfig& config) : config_(config) {
}

Cache::~Cache() {
}

// Order operations
std::vector<const Order*> Cache::orders_open(const StrategyId& strategy_id) const {
    std::vector<const Order*> result;
    for (const auto& pair : orders_) {
        const auto* ord = pair.second.get();
        if (ord->is_open()) {
            if (strategy_id.empty() || ord->strategy_id() == strategy_id) {
                result.push_back(ord);
            }
        }
    }
    return result;
}

std::vector<const Order*> Cache::orders_closed(const StrategyId& strategy_id) const {
    std::vector<const Order*> result;
    for (const auto& pair : orders_) {
        const auto* ord = pair.second.get();
        if (ord->is_closed()) {
            if (strategy_id.empty() || ord->strategy_id() == strategy_id) {
                result.push_back(ord);
            }
        }
    }
    return result;
}

const Order* Cache::order(const OrderId& order_id) const {
    auto it = orders_.find(order_id);
    return (it != orders_.end()) ? it->second.get() : nullptr;
}

bool Cache::order_exists(const OrderId& order_id) const {
    return orders_.find(order_id) != orders_.end();
}

void Cache::add_order(Order order) {
    if (is_stale_and_update(latest_order_ts_, order.order_id(), order.timestamp())) {
        return;
    }
    auto id = order.order_id();
    orders_[id] = std::make_unique<Order>(std::move(order));
}

void Cache::update_order(const Order& order) {
    if (is_stale_and_update(latest_order_ts_, order.order_id(), order.timestamp())) {
        return;
    }
    auto it = orders_.find(order.order_id());
    if (it != orders_.end()) {
        *(it->second) = order;
    } else {
        add_order(order);
    }
}

// Position operations
std::vector<const Position*> Cache::positions_open(const StrategyId& strategy_id) const {
    std::vector<const Position*> result;
    for (const auto& pair : positions_) {
        const auto* pos = pair.second.get();
        if (pos->is_open()) {
            if (strategy_id.empty() || pos->strategy_id() == strategy_id) {
                result.push_back(pos);
            }
        }
    }
    return result;
}

std::vector<const Position*> Cache::positions_closed(const StrategyId& strategy_id) const {
    std::vector<const Position*> result;
    for (const auto& pair : positions_) {
        const auto* pos = pair.second.get();
        if (pos->is_closed()) {
            if (strategy_id.empty() || pos->strategy_id() == strategy_id) {
                result.push_back(pos);
            }
        }
    }
    return result;
}

const Position* Cache::position(const PositionId& position_id) const {
    auto it = positions_.find(position_id);
    return (it != positions_.end()) ? it->second.get() : nullptr;
}

const Position* Cache::position_for_instrument(const InstrumentId& instrument_id,
                                               const StrategyId& strategy_id) const {
    for (const auto& pair : positions_) {
        const auto* pos = pair.second.get();
        if (pos->instrument_id() == instrument_id &&
            pos->is_open()) {
            if (strategy_id.empty() || pos->strategy_id() == strategy_id) {
                return pos;
            }
        }
    }
    return nullptr;
}

bool Cache::position_exists(const PositionId& position_id) const {
    return positions_.find(position_id) != positions_.end();
}

void Cache::add_position(Position position) {
    if (is_stale_and_update(latest_position_ts_, position.position_id(), position.timestamp())) {
        return;
    }
    auto id = position.position_id();
    positions_[id] = std::make_unique<Position>(std::move(position));
}

void Cache::update_position(const Position& position) {
    if (is_stale_and_update(latest_position_ts_, position.position_id(), position.timestamp())) {
        return;
    }
    auto it = positions_.find(position.position_id());
    if (it != positions_.end()) {
        *(it->second) = position;
    } else {
        add_position(position);
    }
}

// Market data operations
const QuoteTick* Cache::quote_tick(const InstrumentId& instrument_id) const {
    auto it = quote_ticks_.find(instrument_id);
    return (it != quote_ticks_.end()) ? &(it->second) : nullptr;
}

const TradeTick* Cache::trade_tick(const InstrumentId& instrument_id) const {
    auto it = trade_ticks_.find(instrument_id);
    return (it != trade_ticks_.end()) ? &(it->second) : nullptr;
}

const Bar* Cache::bar(const BarType& bar_type) const {
    auto key = bar_key(bar_type);
    auto it = bars_.find(key);
    return (it != bars_.end()) ? &(it->second) : nullptr;
}

const OrderBook* Cache::order_book(const InstrumentId& instrument_id,
                                   const BookFrequency& frequency) const {
    auto key = order_book_key(instrument_id, frequency);
    auto it = order_books_.find(key);
    return (it != order_books_.end()) ? &(it->second) : nullptr;
}

std::vector<OrderBook> Cache::recent_order_books(const InstrumentId& instrument_id,
                                                 const BookFrequency& frequency,
                                                 size_t n) const {
    auto key = order_book_key(instrument_id, frequency);
    auto it = order_book_history_.find(key);
    if (it == order_book_history_.end()) {
        return {};
    }
    return it->second.recent(n);
}

void Cache::add_quote_tick(const QuoteTick& tick) {
    if (is_stale_and_update(latest_quote_ts_, tick.instrument_id(), tick.timestamp())) {
        return;
    }
    quote_ticks_[tick.instrument_id()] = tick;
}

void Cache::add_trade_tick(const TradeTick& tick) {
    if (is_stale_and_update(latest_trade_ts_, tick.instrument_id(), tick.timestamp())) {
        return;
    }
    trade_ticks_[tick.instrument_id()] = tick;
    if (config_.trade_capacity > 0) {
        auto [it, inserted] = trade_history_.try_emplace(tick.instrument_id(), config_.trade_capacity);
        it->second.push(tick);
    }
}

void Cache::add_bar(const Bar& bar) {
    auto key = bar_key(bar.bar_type());
    if (is_stale_and_update(latest_bar_ts_, key, bar.timestamp())) {
        return;
    }
    bars_[key] = bar;
    if (config_.bar_capacity > 0) {
        auto [it, inserted] = bar_history_.try_emplace(key, config_.bar_capacity);
        it->second.push(bar);
    }
}

void Cache::update_order_book(const OrderBook& book, const BookFrequency& frequency) {
    auto key = order_book_key(book.instrument_id(), frequency);
    if (is_stale_and_update(latest_order_book_ts_, key, book.timestamp())) {
        return;
    }
    order_books_[key] = book;
    if (config_.orderbook_capacity > 0) {
        auto [it, inserted] = order_book_history_.try_emplace(key, config_.orderbook_capacity);
        it->second.push(book);
    }
}

// Instrument/Account operations
const Instrument* Cache::instrument(const InstrumentId& instrument_id) const {
    auto it = instruments_.find(instrument_id);
    return (it != instruments_.end()) ? &(it->second) : nullptr;
}

void Cache::add_instrument(const Instrument& instrument) {
    if (is_stale_and_update(latest_instrument_ts_, instrument.id(), instrument.timestamp())) {
        return;
    }
    instruments_[instrument.id()] = instrument;
}

const Account* Cache::account(const AccountId& account_id) const {
    auto it = accounts_.find(account_id);
    return (it != accounts_.end()) ? &(it->second) : nullptr;
}

void Cache::add_account(const Account& account) {
    if (is_stale_and_update(latest_account_ts_, account.account_id(), account.timestamp())) {
        return;
    }
    accounts_[account.account_id()] = account;
}

void Cache::clear() {
    orders_.clear();
    positions_.clear();
    quote_ticks_.clear();
    trade_ticks_.clear();
    bars_.clear();
    order_books_.clear();
    instruments_.clear();
    accounts_.clear();
    trade_history_.clear();
    bar_history_.clear();
    order_book_history_.clear();
    latest_quote_ts_.clear();
    latest_trade_ts_.clear();
    latest_bar_ts_.clear();
    latest_order_book_ts_.clear();
    latest_order_ts_.clear();
    latest_position_ts_.clear();
    latest_instrument_ts_.clear();
    latest_account_ts_.clear();
}

void Cache::print_stats() const {
    // TODO: Implement statistics
}

std::vector<QuoteTick> Cache::recent_quotes(const InstrumentId& instrument_id, size_t n) const {
    if (n == 0) {
        return {};
    }
    auto latest = quote_tick(instrument_id);
    if (!latest) {
        return {};
    }
    return { *latest };
}

std::vector<TradeTick> Cache::recent_trades(const InstrumentId& instrument_id, size_t n) const {
    auto it = trade_history_.find(instrument_id);
    if (it == trade_history_.end()) {
        return {};
    }
    return it->second.recent(n);
}

std::vector<Bar> Cache::recent_bars(const BarType& bar_type, size_t n) const {
    auto key = bar_key(bar_type);
    auto it = bar_history_.find(key);
    if (it == bar_history_.end()) {
        return {};
    }
    return it->second.recent(n);
}

} // namespace npcTrading
