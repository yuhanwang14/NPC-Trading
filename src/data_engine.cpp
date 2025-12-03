#include "npcTrading/data_engine.hpp"

namespace npcTrading {

DataEngine::DataEngine(MessageBus* msgbus, Cache* cache, Clock* clock,
                       const DataEngineConfig& config)
    : Component("DataEngine", msgbus, cache, clock), config_(config) {
}

void DataEngine::initialize() {
    Component::initialize();
    
    // Register message handlers
    msgbus_->register_handler(
        Endpoints::DATA_ENGINE_EXECUTE,
        [this](const std::shared_ptr<Message>& msg) { handle_execute(msg); }
    );
    
    msgbus_->register_handler(
        Endpoints::DATA_ENGINE_PROCESS,
        [this](const std::shared_ptr<Message>& msg) { handle_process(msg); }
    );
    
    msgbus_->register_request_handler(
        Endpoints::DATA_ENGINE_REQUEST,
        [this](const std::shared_ptr<Request>& req) { return handle_request(req); }
    );
    
    msgbus_->register_handler(
        Endpoints::DATA_ENGINE_RESPONSE,
        [this](const std::shared_ptr<Message>& msg) { handle_response(msg); }
    );
}

void DataEngine::start() {
    Component::start();
    // TODO: Start all registered clients
}

void DataEngine::stop() {
    Component::stop();
    // TODO: Stop all registered clients
}

void DataEngine::register_client(std::shared_ptr<DataClient> client) {
    clients_[client->client_id()] = client;
    venue_to_client_[client->venue()] = client->client_id();
}

DataClient* DataEngine::get_client(const ClientId& client_id) const {
    auto it = clients_.find(client_id);
    return (it != clients_.end()) ? it->second.get() : nullptr;
}

DataClient* DataEngine::get_client_for_venue(const VenueId& venue) const {
    auto it = venue_to_client_.find(venue);
    if (it != venue_to_client_.end()) {
        return get_client(it->second);
    }
    return nullptr;
}

void DataEngine::handle_execute(const std::shared_ptr<Message>& msg) {
    // TODO: Handle subscription commands
    log_debug("Received execute command: " + msg->type());
}

void DataEngine::handle_process(const std::shared_ptr<Message>& msg) {
    // TODO: Handle incoming market data
    log_debug("Received data: " + msg->type());
}

std::shared_ptr<Response> DataEngine::handle_request(const std::shared_ptr<Request>& req) {
    // TODO: Handle data requests
    log_debug("Received request: " + req->type());
    return nullptr;
}

void DataEngine::handle_response(const std::shared_ptr<Message>& msg) {
    // TODO: Handle data responses
    log_debug("Received response: " + msg->type());
}

// LiveDataEngine implementation
LiveDataEngine::LiveDataEngine(MessageBus* msgbus, Cache* cache, Clock* clock,
                               const LiveDataEngineConfig& config)
    : DataEngine(msgbus, cache, clock, config), live_config_(config) {
}

void LiveDataEngine::start() {
    DataEngine::start();
    // TODO: Initialize live-specific features
}

void LiveDataEngine::stop() {
    DataEngine::stop();
    // TODO: Cleanup live-specific features
}

} // namespace npcTrading
