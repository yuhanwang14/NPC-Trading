#include "npcTrading/execution_engine.hpp"

namespace npcTrading {

ExecutionEngine::ExecutionEngine(MessageBus* msgbus, Cache* cache, Clock* clock,
                                 const ExecEngineConfig& config)
    : Component("ExecutionEngine", msgbus, cache, clock), config_(config) {
}

void ExecutionEngine::on_initialize() {
    // Register message handlers
    msgbus_->register_handler(
        Endpoints::EXEC_ENGINE_EXECUTE,
        [this](const std::shared_ptr<Message>& msg) { handle_execute(msg); }
    );

    msgbus_->register_handler(
        Endpoints::EXEC_ENGINE_PROCESS,
        [this](const std::shared_ptr<Message>& msg) { handle_process(msg); }
    );
}

void ExecutionEngine::register_client(std::shared_ptr<ExecutionClient> client) {
    clients_[client->client_id()] = client;
    venue_to_client_[client->venue()] = client->client_id();
}

ExecutionClient* ExecutionEngine::get_client(const ClientId& client_id) const {
    auto it = clients_.find(client_id);
    return (it != clients_.end()) ? it->second.get() : nullptr;
}

ExecutionClient* ExecutionEngine::get_client_for_venue(const VenueId& venue) const {
    auto it = venue_to_client_.find(venue);
    if (it != venue_to_client_.end()) {
        return get_client(it->second);
    }
    return nullptr;
}

void ExecutionEngine::handle_execute(const std::shared_ptr<Message>& msg) {
    // TODO: Handle trading commands
    log_debug("Received execute command: " + msg->type());
}

void ExecutionEngine::handle_process(const std::shared_ptr<Message>& msg) {
    // TODO: Handle order events
    log_debug("Received order event: " + msg->type());
}

void ExecutionEngine::handle_fill(const OrderFilled& event) {
    // TODO: Update position
}

void ExecutionEngine::update_position(Order* order, const Fill& fill) {
    // TODO: Implement position update
}

PositionId ExecutionEngine::generate_position_id(Order* order) {
    // TODO: Generate position ID based on OMS type
    return order->instrument_id() + "-" + order->strategy_id();
}

// LiveExecutionEngine implementation
LiveExecutionEngine::LiveExecutionEngine(MessageBus* msgbus, Cache* cache, Clock* clock,
                                         const LiveExecEngineConfig& config)
    : ExecutionEngine(msgbus, cache, clock, config), live_config_(config) {
}

void LiveExecutionEngine::on_start() {
    ExecutionEngine::on_start();
    // TODO: Live-specific start logic
}

void LiveExecutionEngine::on_stop() {
    ExecutionEngine::on_stop();
    // TODO: Live-specific stop logic
}

} // namespace npcTrading
