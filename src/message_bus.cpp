#include "npcTrading/message_bus.hpp"
#include <iostream>

namespace npcTrading {

MessageBus::MessageBus(const MessageBusConfig& config)
    : config_(config), running_(false) {
    // TODO: Initialize persistence if enabled
}

MessageBus::~MessageBus() {
    if (running_) {
        stop();
    }
}

void MessageBus::register_handler(const std::string& endpoint, MessageHandler handler) {
    message_handlers_[endpoint] = handler;
}

void MessageBus::register_request_handler(const std::string& endpoint, RequestHandler handler) {
    request_handlers_[endpoint] = handler;
}

void MessageBus::unregister_handler(const std::string& endpoint) {
    message_handlers_.erase(endpoint);
    request_handlers_.erase(endpoint);
}

void MessageBus::send(const std::string& endpoint, std::shared_ptr<Message> message) {
    auto it = message_handlers_.find(endpoint);
    if (it != message_handlers_.end()) {
        it->second(message);
    } else {
        std::cerr << "No handler registered for endpoint: " << endpoint << std::endl;
    }
}

std::shared_ptr<Response> MessageBus::request(const std::string& endpoint, 
                                              std::shared_ptr<Request> request) {
    auto it = request_handlers_.find(endpoint);
    if (it != request_handlers_.end()) {
        return it->second(request);
    } else {
        std::cerr << "No request handler registered for endpoint: " << endpoint << std::endl;
        return nullptr;
    }
}

void MessageBus::subscribe(const std::string& topic, MessageHandler handler) {
    subscribers_[topic].push_back(handler);
}

void MessageBus::unsubscribe(const std::string& topic) {
    subscribers_.erase(topic);
}

void MessageBus::publish(const std::string& topic, std::shared_ptr<Message> message) {
    auto it = subscribers_.find(topic);
    if (it != subscribers_.end()) {
        for (const auto& handler : it->second) {
            handler(message);
        }
    }
}

void MessageBus::start() {
    running_ = true;
}

void MessageBus::stop() {
    running_ = false;
}

} // namespace npcTrading
