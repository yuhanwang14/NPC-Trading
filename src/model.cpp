#include "npcTrading/model.hpp"
#include <iostream>

namespace npcTrading {

// Order implementation
bool Order::is_open() const {
    return status_ == OrderStatus::INITIALIZED ||
           status_ == OrderStatus::SUBMITTED ||
           status_ == OrderStatus::ACCEPTED ||
           status_ == OrderStatus::TRIGGERED ||
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
    // STUB-WITH-GUARD: Not implemented — log warning so callers know P&L is wrong
    std::cerr << "[Position] WARNING: apply_fill() NOT IMPLEMENTED — position "
              << position_id_ << " will not reflect fill for order " << fill.order_id()
              << ". P&L data is UNRELIABLE." << std::endl;
    // TODO: Implement position update logic with proper P&L calculation
}

void Position::update_unrealized_pnl(Price current_price) {
    // STUB-WITH-GUARD: Not implemented — unrealized P&L will always be zero
    std::cerr << "[Position] WARNING: update_unrealized_pnl() NOT IMPLEMENTED — position "
              << position_id_ << " unrealized P&L is STALE." << std::endl;
    // TODO: Calculate unrealized P&L based on current price
}

} // namespace npcTrading
