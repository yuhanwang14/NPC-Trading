#include <gtest/gtest.h>
#include "npcTrading/cache.hpp"
#include "npcTrading/market_data.hpp"
#include "npcTrading/model.hpp"

using namespace npcTrading;

namespace {
Timestamp ts(int seconds) {
    return std::chrono::system_clock::time_point(std::chrono::seconds(seconds));
}
} // namespace

TEST(CacheTest, OrderAndPositionTimeFiltering) {
    Cache cache;
    Order o1("O1", "S1", "INSTR", "CL1", OrderSide::BUY, OrderType::MARKET, Quantity(1), Price(0), TimeInForce::GTC, ts(1));
    cache.add_order(o1);
    // Older update should be dropped
    Order o_old("O1", "S1", "INSTR", "CL1", OrderSide::BUY, OrderType::MARKET, Quantity(2), Price(0), TimeInForce::GTC, ts(0));
    cache.update_order(o_old);
    const Order* stored = cache.order("O1");
    ASSERT_NE(stored, nullptr);
    EXPECT_DOUBLE_EQ(stored->quantity().as_double(), 1.0);
    EXPECT_EQ(stored->timestamp(), ts(1));

    Position p1("P1", "INSTR", "S1", ts(2));
    cache.add_position(p1);
    Position p_newer("P1", "INSTR", "S1", ts(3));
    cache.update_position(p_newer);
    const Position* pos = cache.position("P1");
    ASSERT_NE(pos, nullptr);
    EXPECT_EQ(pos->timestamp(), ts(3));
}

TEST(CacheTest, QuoteStoresLatestOnlyAndDropsStale) {
    Cache cache;
    QuoteTick q1("INSTR", Price(10), Price(11), Quantity(1), Quantity(1), ts(5));
    QuoteTick q_old("INSTR", Price(9), Price(10), Quantity(1), Quantity(1), ts(4));
    cache.add_quote_tick(q1);
    cache.add_quote_tick(q_old); // stale, should be ignored
    const QuoteTick* latest = cache.quote_tick("INSTR");
    ASSERT_NE(latest, nullptr);
    EXPECT_EQ(latest->timestamp(), ts(5));
    auto recent = cache.recent_quotes("INSTR", 5);
    ASSERT_EQ(recent.size(), 1u);
    EXPECT_EQ(recent[0].timestamp(), ts(5));
}

TEST(CacheTest, TradeHistoryRespectsCapacityAndStaleness) {
    CacheConfig cfg;
    cfg.trade_capacity = 2;
    Cache cache(cfg);
    cache.add_trade_tick(TradeTick("INSTR", Price(1), Quantity(1), OrderSide::BUY, ts(1)));
    cache.add_trade_tick(TradeTick("INSTR", Price(2), Quantity(1), OrderSide::BUY, ts(2)));
    // This one is stale and ignored
    cache.add_trade_tick(TradeTick("INSTR", Price(0.5), Quantity(1), OrderSide::BUY, ts(1)));
    // Capacity drop: only keep last two (ts2, ts3)
    cache.add_trade_tick(TradeTick("INSTR", Price(3), Quantity(1), OrderSide::BUY, ts(3)));
    auto recent = cache.recent_trades("INSTR", 5);
    ASSERT_EQ(recent.size(), 2u);
    EXPECT_EQ(recent[0].timestamp(), ts(2));
    EXPECT_EQ(recent[1].timestamp(), ts(3));
}

TEST(CacheTest, BarHistoryWithCapacityAndStaleness) {
    CacheConfig cfg;
    cfg.bar_capacity = 2;
    Cache cache(cfg);
    BarType type("BTCUSDT", "1m");
    cache.add_bar(Bar(type, Price(1), Price(2), Price(0.5), Price(1.5), Quantity(10), ts(10)));
    cache.add_bar(Bar(type, Price(2), Price(3), Price(1.5), Price(2.5), Quantity(15), ts(20)));
    // Stale vs latest
    cache.add_bar(Bar(type, Price(0), Price(0), Price(0), Price(0), Quantity(0), ts(15)));
    // Capacity drop keeps last two by time
    cache.add_bar(Bar(type, Price(3), Price(4), Price(2.5), Price(3.5), Quantity(20), ts(30)));
    auto bars = cache.recent_bars(type, 10);
    ASSERT_EQ(bars.size(), 2u);
    EXPECT_EQ(bars[0].timestamp(), ts(20));
    EXPECT_EQ(bars[1].timestamp(), ts(30));
}

TEST(CacheTest, OrderBookPerFrequencyAndHistory) {
    CacheConfig cfg;
    cfg.orderbook_capacity = 2;
    Cache cache(cfg);
    OrderBookLevel lvl{Price(10), Quantity(1), 1};
    OrderBook book_fast("BTC", {lvl}, {lvl}, ts(5));
    OrderBook book_slow("BTC", {lvl}, {lvl}, ts(5));
    cache.update_order_book(book_fast, "0.5s");
    cache.update_order_book(book_slow, "10s");
    // Same instrument, different frequency both stored
    auto* fast = cache.order_book("BTC", "0.5s");
    auto* slow = cache.order_book("BTC", "10s");
    ASSERT_NE(fast, nullptr);
    ASSERT_NE(slow, nullptr);
    EXPECT_EQ(fast->timestamp(), ts(5));
    EXPECT_EQ(slow->timestamp(), ts(5));

    // Stale update on fast freq should be ignored
    cache.update_order_book(OrderBook("BTC", {lvl}, {lvl}, ts(4)), "0.5s");
    fast = cache.order_book("BTC", "0.5s");
    EXPECT_EQ(fast->timestamp(), ts(5));

    // History per frequency
    cache.update_order_book(OrderBook("BTC", {lvl}, {lvl}, ts(6)), "0.5s");
    auto history = cache.recent_order_books("BTC", "0.5s", 5);
    ASSERT_EQ(history.size(), 2u);
    EXPECT_EQ(history[0].timestamp(), ts(5));
    EXPECT_EQ(history[1].timestamp(), ts(6));
}

TEST(CacheTest, InstrumentAndAccountTimeFiltering) {
    Cache cache;
    Instrument instr_new("I1", "SYM", "VEN", 0.01, 0.001, 0.1, 100, ts(10));
    Instrument instr_old("I1", "SYM", "VEN", 0.01, 0.001, 0.1, 100, ts(5));
    cache.add_instrument(instr_new);
    cache.add_instrument(instr_old); // stale
    const Instrument* instr = cache.instrument("I1");
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(instr->timestamp(), ts(10));

    Account acc_new("A1", ts(20));
    Account acc_old("A1", ts(15));
    cache.add_account(acc_new);
    cache.add_account(acc_old); // stale
    const Account* acc = cache.account("A1");
    ASSERT_NE(acc, nullptr);
    EXPECT_EQ(acc->timestamp(), ts(20));
}

TEST(CacheTest, ClearResetsAllStoredData) {
    CacheConfig cfg;
    cfg.trade_capacity = 2;
    cfg.bar_capacity = 2;
    cfg.orderbook_capacity = 2;
    Cache cache(cfg);
    cache.add_order(Order("O1", "S1", "INSTR", "CL1", OrderSide::BUY, OrderType::MARKET, Quantity(1), Price(0), TimeInForce::GTC, ts(1)));
    cache.add_position(Position("P1", "INSTR", "S1", ts(1)));
    cache.add_quote_tick(QuoteTick("INSTR", Price(1), Price(2), Quantity(1), Quantity(1), ts(1)));
    cache.add_trade_tick(TradeTick("INSTR", Price(1), Quantity(1), OrderSide::BUY, ts(1)));
    BarType type("INSTR", "1m");
    cache.add_bar(Bar(type, Price(1), Price(2), Price(0.5), Price(1.5), Quantity(1), ts(1)));
    OrderBookLevel lvl{Price(1), Quantity(1), 1};
    cache.update_order_book(OrderBook("INSTR", {lvl}, {lvl}, ts(1)), "fast");
    cache.add_instrument(Instrument("INSTR", "SYM", "VEN", 0.01, 0.001, 0.1, 100, ts(1)));
    cache.add_account(Account("ACC", ts(1)));

    cache.clear();

    EXPECT_EQ(cache.order("O1"), nullptr);
    EXPECT_EQ(cache.position("P1"), nullptr);
    EXPECT_EQ(cache.quote_tick("INSTR"), nullptr);
    EXPECT_EQ(cache.trade_tick("INSTR"), nullptr);
    EXPECT_EQ(cache.bar(type), nullptr);
    EXPECT_EQ(cache.order_book("INSTR", "fast"), nullptr);
    EXPECT_EQ(cache.instrument("INSTR"), nullptr);
    EXPECT_EQ(cache.account("ACC"), nullptr);
    EXPECT_TRUE(cache.recent_trades("INSTR", 5).empty());
    EXPECT_TRUE(cache.recent_bars(type, 5).empty());
    EXPECT_TRUE(cache.recent_order_books("INSTR", "fast", 5).empty());
}
