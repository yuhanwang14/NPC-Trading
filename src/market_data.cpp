#include "npcTrading/market_data.hpp"
#include "npcTrading/strategy.hpp"

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

} // namespace npcTrading
