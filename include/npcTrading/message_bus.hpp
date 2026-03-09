#pragma once

#include "common.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <string>
#include <any>
#include <queue>
#include <deque>
#include <mutex>
#include <atomic>
#include <stdexcept>

namespace npcTrading {

/// Token returned from subscribe() to enable per-subscriber unsubscribe
using SubscriptionToken = std::uint64_t;

/// Invalid token constant
constexpr SubscriptionToken INVALID_SUBSCRIPTION_TOKEN = 0;

// Forward declarations
class Message;
class Request;
class Response;

// ============================================================================
// Message Types
// ============================================================================

/// Base message class
class Message {
public:
    virtual ~Message() = default;
    
    virtual Timestamp timestamp() const = 0;
    virtual std::string type() const = 0;
};

/// Base request class
class Request : public Message {
public:
    virtual ~Request() = default;
};

/// Base response class
class Response : public Message {
public:
    virtual ~Response() = default;
};

// ============================================================================
// MessageBus Configuration
// ============================================================================

struct MessageBusConfig {
    std::string database_path;
    bool persistence_enabled = false;
    size_t max_queue_size = 10000;
};

// ============================================================================
// MessageBus - Global event routing system
// ============================================================================

/**
 * @brief Global single-threaded event bus for pub/sub and req/rep patterns
 * 
 * The MessageBus is the central nervous system of the trading platform.
 * All components communicate through registered endpoints.
 * 
 * Supported patterns:
 * - Pub/Sub: send() for async fire-and-forget
 * - Req/Rep: request() for sync request-response
 * - Point-to-point: Direct endpoint routing
 */
class MessageBus {
public:
    using MessageHandler = std::function<void(const std::shared_ptr<Message>&)>;
    using RequestHandler = std::function<std::shared_ptr<Response>(const std::shared_ptr<Request>&)>;
    
    explicit MessageBus(const MessageBusConfig& config = MessageBusConfig());
    ~MessageBus();
    
    // ========================================================================
    // Endpoint Registration
    // ========================================================================
    
    /**
     * @brief Register a message handler for an endpoint
     * @param endpoint Endpoint name (e.g., "DataEngine.process")
     * @param handler Callback function to handle messages
     */
    void register_handler(const std::string& endpoint, MessageHandler handler);
    
    /**
     * @brief Register a request handler for an endpoint
     * @param endpoint Endpoint name (e.g., "DataEngine.request")
     * @param handler Callback function to handle requests and return responses
     */
    void register_request_handler(const std::string& endpoint, RequestHandler handler);
    
    /**
     * @brief Unregister a handler from an endpoint
     * @param endpoint Endpoint name
     */
    void unregister_handler(const std::string& endpoint);
    
    // ========================================================================
    // Message Sending
    // ========================================================================
    
    /**
     * @brief Send a message asynchronously (fire-and-forget)
     * @param endpoint Target endpoint
     * @param message Message to send
     */
    void send(const std::string& endpoint, std::shared_ptr<Message> message);
    
    /**
     * @brief Send a request synchronously and wait for response
     * @param endpoint Target endpoint
     * @param request Request to send
     * @return Response from the handler
     */
    std::shared_ptr<Response> request(const std::string& endpoint, 
                                      std::shared_ptr<Request> request);
    
    // ========================================================================
    // Publishing (multiple subscribers)
    // ========================================================================
    
    /**
     * @brief Subscribe to a topic
     * @param topic Topic name
     * @param handler Callback for published messages
     * @return SubscriptionToken that can be used to unsubscribe just this handler
     */
    SubscriptionToken subscribe(const std::string& topic, MessageHandler handler);
    
    /**
     * @brief Unsubscribe a specific subscription by token
     * @param token Token returned from subscribe()
     */
    void unsubscribe(SubscriptionToken token);
    
    /**
     * @brief Unsubscribe all handlers from a topic
     * @param topic Topic name
     */
    void unsubscribe_all(const std::string& topic);
    
    /**
     * @brief Publish a message to all subscribers of a topic
     * @param topic Topic name
     * @param message Message to publish
     */
    void publish(const std::string& topic, std::shared_ptr<Message> message);
    
    // ========================================================================
    // Lifecycle
    // ========================================================================
    
    void start();
    void stop();
    void run();  // Process queued messages (event loop)
    bool is_running() const { return running_; }
    
    // ========================================================================
    // Statistics
    // ========================================================================
    
    size_t queue_size() const;
    size_t total_messages_processed() const { return messages_processed_; }
    
private:
    struct QueuedMessage {
        std::string endpoint_or_topic;
        std::shared_ptr<Message> message;
        bool is_publish;  // true for publish, false for send
    };
    
    MessageBusConfig config_;
    std::atomic<bool> running_;
    
    // Endpoint handlers (point-to-point)
    std::unordered_map<std::string, MessageHandler> message_handlers_;
    std::unordered_map<std::string, RequestHandler> request_handlers_;
    
    // Topic subscribers (pub/sub) - token-based for per-subscriber unsubscribe
    // topic -> (token -> handler)
    std::unordered_map<std::string, std::unordered_map<SubscriptionToken, MessageHandler>> subscribers_;
    // token -> topic (reverse lookup for O(1) unsubscribe by token)
    std::unordered_map<SubscriptionToken, std::string> token_to_topic_;
    // Next token to assign
    SubscriptionToken next_token_ = 1;
    // Mutex for subscriber structures
    mutable std::mutex sub_mutex_;
    
    // Message queue for async processing
    std::deque<QueuedMessage> message_queue_;
    mutable std::mutex queue_mutex_;  // For thread-safe queue access
    
    // Statistics
    size_t messages_processed_ = 0;
    
    // Internal helpers
    void process_message(const QueuedMessage& msg);
};

// ============================================================================
// Standard Endpoints (convention)
// ============================================================================

namespace Endpoints {
    // Data Engine
    constexpr const char* DATA_ENGINE_EXECUTE = "DataEngine.execute";
    constexpr const char* DATA_ENGINE_PROCESS = "DataEngine.process";
    constexpr const char* DATA_ENGINE_PROCESS_HISTORICAL = "DataEngine.process_historical";
    constexpr const char* DATA_ENGINE_REQUEST = "DataEngine.request";
    constexpr const char* DATA_ENGINE_RESPONSE = "DataEngine.response";
    
    // Execution Engine
    constexpr const char* EXEC_ENGINE_EXECUTE = "ExecEngine.execute";
    constexpr const char* EXEC_ENGINE_PROCESS = "ExecEngine.process";
    
    // Risk Engine
    constexpr const char* RISK_ENGINE_EXECUTE = "RiskEngine.execute";
    constexpr const char* RISK_ENGINE_PROCESS = "RiskEngine.process";
}

} // namespace npcTrading
