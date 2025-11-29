#include "npcTrading/message_bus.hpp"
#include <iostream>
#include <algorithm>

namespace npcTrading {

// ============================================================================
// Construction / Destruction
// ============================================================================

MessageBus::MessageBus(const MessageBusConfig& config)
    : config_(config), running_(false), messages_processed_(0) {
    if (config_.max_queue_size == 0) {
        throw std::invalid_argument("max_queue_size must be greater than 0");
    }
}

MessageBus::~MessageBus() {
    if (running_) {
        stop();
    }
}

// ============================================================================
// Endpoint Registration
// ============================================================================

void MessageBus::register_handler(const std::string& endpoint, MessageHandler handler) {
    if (endpoint.empty()) {
        throw std::invalid_argument("Endpoint name cannot be empty");
    }
    if (!handler) {
        throw std::invalid_argument("Handler cannot be null");
    }
    message_handlers_[endpoint] = handler;
}

void MessageBus::register_request_handler(const std::string& endpoint, RequestHandler handler) {
    if (endpoint.empty()) {
        throw std::invalid_argument("Endpoint name cannot be empty");
    }
    if (!handler) {
        throw std::invalid_argument("Handler cannot be null");
    }
    request_handlers_[endpoint] = handler;
}

void MessageBus::unregister_handler(const std::string& endpoint) {
    message_handlers_.erase(endpoint);
    request_handlers_.erase(endpoint);
}

// ============================================================================
// Message Sending (Async)
// ============================================================================

void MessageBus::send(const std::string& endpoint, std::shared_ptr<Message> message) {
    if (!message) {
        throw std::invalid_argument("Message cannot be null");
    }
    
    if (!running_) {
        std::cerr << "[MessageBus] Warning: Cannot send message - bus not running" << std::endl;
        return;
    }
    
    // Queue the message for async processing
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    if (message_queue_.size() >= config_.max_queue_size) {
        std::cerr << "[MessageBus] Warning: Queue full (" << config_.max_queue_size 
                  << "), dropping message to endpoint: " << endpoint << std::endl;
        return;
    }
    
    message_queue_.push_back({endpoint, message, false});
}

// ============================================================================
// Request-Response (Sync)
// ============================================================================

std::shared_ptr<Response> MessageBus::request(const std::string& endpoint, 
                                              std::shared_ptr<Request> request) {
    if (!request) {
        throw std::invalid_argument("Request cannot be null");
    }
    
    auto it = request_handlers_.find(endpoint);
    if (it != request_handlers_.end()) {
        return it->second(request);
    } else {
        std::cerr << "[MessageBus] Error: No request handler registered for endpoint: " 
                  << endpoint << std::endl;
        return nullptr;
    }
}

// ============================================================================
// Pub/Sub
// ============================================================================

void MessageBus::subscribe(const std::string& topic, MessageHandler handler) {
    if (topic.empty()) {
        throw std::invalid_argument("Topic name cannot be empty");
    }
    if (!handler) {
        throw std::invalid_argument("Handler cannot be null");
    }
    subscribers_[topic].push_back(handler);
}

void MessageBus::unsubscribe(const std::string& topic) {
    subscribers_.erase(topic);
}

void MessageBus::publish(const std::string& topic, std::shared_ptr<Message> message) {
    if (!message) {
        throw std::invalid_argument("Message cannot be null");
    }
    
    if (!running_) {
        std::cerr << "[MessageBus] Warning: Cannot publish message - bus not running" << std::endl;
        return;
    }
    
    // Queue the message for async processing
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    if (message_queue_.size() >= config_.max_queue_size) {
        std::cerr << "[MessageBus] Warning: Queue full (" << config_.max_queue_size 
                  << "), dropping message to topic: " << topic << std::endl;
        return;
    }
    
    message_queue_.push_back({topic, message, true});
}

// ============================================================================
// Lifecycle
// ============================================================================

void MessageBus::start() {
    if (running_) {
        std::cerr << "[MessageBus] Warning: Already running" << std::endl;
        return;
    }
    running_ = true;
    std::cout << "[MessageBus] Started" << std::endl;
}

void MessageBus::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // Process remaining messages in queue
    std::lock_guard<std::mutex> lock(queue_mutex_);
    size_t remaining = message_queue_.size();
    if (remaining > 0) {
        std::cout << "[MessageBus] Processing " << remaining 
                  << " remaining messages before stopping..." << std::endl;
        while (!message_queue_.empty()) {
            process_message(message_queue_.front());
            message_queue_.pop_front();
        }
    }
    
    std::cout << "[MessageBus] Stopped (processed " << messages_processed_ 
              << " messages total)" << std::endl;
}

void MessageBus::run() {
    if (!running_) {
        throw std::runtime_error("MessageBus must be started before calling run()");
    }
    
    // Event loop: process all queued messages
    while (running_) {
        QueuedMessage msg;
        bool has_message = false;
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (!message_queue_.empty()) {
                msg = message_queue_.front();
                message_queue_.pop_front();
                has_message = true;
            }
        }
        
        if (has_message) {
            process_message(msg);
        } else {
            // No messages - yield CPU briefly
            // In production, might use condition variables here
            // For now, just break if queue is empty (single iteration mode)
            break;
        }
    }
}

// ============================================================================
// Statistics
// ============================================================================

size_t MessageBus::queue_size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return message_queue_.size();
}

// ============================================================================
// Internal Helpers
// ============================================================================

void MessageBus::process_message(const QueuedMessage& msg) {
    messages_processed_++;  // Count before processing (even if it fails)
    
    try {
        if (msg.is_publish) {
            // Publish to all subscribers
            auto it = subscribers_.find(msg.endpoint_or_topic);
            if (it != subscribers_.end()) {
                for (const auto& handler : it->second) {
                    handler(msg.message);
                }
            }
        } else {
            // Send to single endpoint
            auto it = message_handlers_.find(msg.endpoint_or_topic);
            if (it != message_handlers_.end()) {
                it->second(msg.message);
            } else {
                std::cerr << "[MessageBus] Warning: No handler for endpoint: " 
                          << msg.endpoint_or_topic << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[MessageBus] Error processing message: " << e.what() << std::endl;
    }
}

} // namespace npcTrading
