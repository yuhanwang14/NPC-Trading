#include "npcTrading/model.hpp"

namespace npcTrading {

// Order implementation
bool Order::is_open() const {
    return status_ == OrderStatus::SUBMITTED ||
           status_ == OrderStatus::ACCEPTED ||
           status_ == OrderStatus::PARTIALLY_FILLED;
}

bool Order::is_closed() const {
    return status_ == OrderStatus::FILLED ||
           status_ == OrderStatus::CANCELED ||
           status_ == OrderStatus::REJECTED ||
           status_ == OrderStatus::EXPIRED;
}

// Position implementation
void Position::apply_fill(const Fill& fill) {
    // TODO: Implement position update logic with proper P&L calculation
}

void Position::update_unrealized_pnl(Price current_price) {
    // TODO: Calculate unrealized P&L based on current price
}

} // namespace npcTrading
