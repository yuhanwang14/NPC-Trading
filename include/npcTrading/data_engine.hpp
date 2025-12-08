#pragma once

#include "common.hpp"
#include "component.hpp"
#include "market_data.hpp"
#include "message_bus.hpp"
#include <memory>
#include <unordered_map>
#include <utility>

namespace npcTrading {

// Forward declarations
class DataClient;

// ============================================================================
// Data Commands (messages sent to DataEngine.execute)
// ============================================================================

class DataCommand : public Message {
public:
    virtual ~DataCommand() = default;
};

class SubscribeBars : public DataCommand {
public:
    SubscribeBars(BarType bar_type, ClientId client_id = "")
        : bar_type_(bar_type), client_id_(client_id) {}
    
    BarType bar_type() const { return bar_type_; }
    ClientId client_id() const { return client_id_; }
    
    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "SubscribeBars"; }
    
private:
    BarType bar_type_;
    ClientId client_id_;
};

class UnsubscribeBars : public DataCommand {
public:
    explicit UnsubscribeBars(BarType bar_type) : bar_type_(bar_type) {}
    
    BarType bar_type() const { return bar_type_; }
    
    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "UnsubscribeBars"; }
    
private:
    BarType bar_type_;
};

class SubscribeQuotes : public DataCommand {
public:
    explicit SubscribeQuotes(InstrumentId instrument_id, ClientId client_id = "")
        : instrument_id_(std::move(instrument_id)), client_id_(std::move(client_id)) {}

    InstrumentId instrument_id() const { return instrument_id_; }
    ClientId client_id() const { return client_id_; }

    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "SubscribeQuotes"; }

private:
    InstrumentId instrument_id_;
    ClientId client_id_;
};

class UnsubscribeQuotes : public DataCommand {
public:
    explicit UnsubscribeQuotes(InstrumentId instrument_id)
        : instrument_id_(std::move(instrument_id)) {}

    InstrumentId instrument_id() const { return instrument_id_; }

    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "UnsubscribeQuotes"; }

private:
    InstrumentId instrument_id_;
};

class SubscribeTrades : public DataCommand {
public:
    explicit SubscribeTrades(InstrumentId instrument_id, ClientId client_id = "")
        : instrument_id_(std::move(instrument_id)), client_id_(std::move(client_id)) {}

    InstrumentId instrument_id() const { return instrument_id_; }
    ClientId client_id() const { return client_id_; }

    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "SubscribeTrades"; }

private:
    InstrumentId instrument_id_;
    ClientId client_id_;
};

class UnsubscribeTrades : public DataCommand {
public:
    explicit UnsubscribeTrades(InstrumentId instrument_id)
        : instrument_id_(std::move(instrument_id)) {}

    InstrumentId instrument_id() const { return instrument_id_; }

    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "UnsubscribeTrades"; }

private:
    InstrumentId instrument_id_;
};

class SubscribeOrderBook : public DataCommand {
public:
    SubscribeOrderBook(InstrumentId instrument_id, int depth = 10, ClientId client_id = "")
        : instrument_id_(std::move(instrument_id)), depth_(depth), client_id_(std::move(client_id)) {}

    InstrumentId instrument_id() const { return instrument_id_; }
    int depth() const { return depth_; }
    ClientId client_id() const { return client_id_; }

    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "SubscribeOrderBook"; }

private:
    InstrumentId instrument_id_;
    int depth_;
    ClientId client_id_;
};

class UnsubscribeOrderBook : public DataCommand {
public:
    explicit UnsubscribeOrderBook(InstrumentId instrument_id)
        : instrument_id_(std::move(instrument_id)) {}

    InstrumentId instrument_id() const { return instrument_id_; }

    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "UnsubscribeOrderBook"; }

private:
    InstrumentId instrument_id_;
};

// ============================================================================
// Data Requests (sent to DataEngine.request)
// ============================================================================

class DataRequest : public Request {
public:
    virtual ~DataRequest() = default;
};

class RequestInstrument : public DataRequest {
public:
    RequestInstrument(InstrumentId instrument_id, VenueId venue)
        : instrument_id_(instrument_id), venue_(venue) {}
    
    InstrumentId instrument_id() const { return instrument_id_; }
    VenueId venue() const { return venue_; }
    
    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "RequestInstrument"; }
    
private:
    InstrumentId instrument_id_;
    VenueId venue_;
};

class RequestBarsRange : public DataRequest {
public:
    RequestBarsRange(BarType bar_type, Timestamp start, Timestamp end)
        : bar_type_(bar_type), start_(start), end_(end) {}
    
    BarType bar_type() const { return bar_type_; }
    Timestamp start() const { return start_; }
    Timestamp end() const { return end_; }
    
    Timestamp timestamp() const override { return std::chrono::system_clock::now(); }
    std::string type() const override { return "RequestBarsRange"; }
    
private:
    BarType bar_type_;
    Timestamp start_;
    Timestamp end_;
};

// ============================================================================
// Data Responses
// ============================================================================

class DataResponse : public Response {
public:
    virtual ~DataResponse() = default;
};

// TODO: Define specific response types

// ============================================================================
// DataEngine Configuration
// ============================================================================

struct DataEngineConfig {
    bool time_bars_build_with_no_updates = true;
    bool time_bars_timestamp_on_close = true;
    bool validate_data_sequence = true;
};

struct LiveDataEngineConfig : public DataEngineConfig {
    bool auto_reconnect = true;
    int reconnect_delay_ms = 5000;
};

// ============================================================================
// DataClient Interface
// ============================================================================

/**
 * @brief Abstract interface for exchange/data provider adapters
 */
class DataClient {
public:
    DataClient(ClientId client_id, VenueId venue)
        : client_id_(client_id), venue_(venue) {}
    
    virtual ~DataClient() = default;
    
    ClientId client_id() const { return client_id_; }
    VenueId venue() const { return venue_; }
    
    // Subscription methods
    virtual void subscribe_bars(const BarType& bar_type) = 0;
    virtual void unsubscribe_bars(const BarType& bar_type) = 0;
    virtual void subscribe_quotes(const InstrumentId& instrument_id) = 0;
    virtual void unsubscribe_quotes(const InstrumentId& instrument_id) = 0;
    virtual void subscribe_trades(const InstrumentId& instrument_id) = 0;
    virtual void unsubscribe_trades(const InstrumentId& instrument_id) = 0;
    virtual void subscribe_order_book(const InstrumentId& instrument_id, int depth = 10) = 0;
    virtual void unsubscribe_order_book(const InstrumentId& instrument_id) = 0;
    
    // Request methods
    virtual void request_instrument(const InstrumentId& instrument_id) = 0;
    virtual void request_bars(const BarType& bar_type, Timestamp start, Timestamp end) = 0;
    
    // Connection management
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
    
protected:
    ClientId client_id_;
    VenueId venue_;
};

// ============================================================================
// DataEngine
// ============================================================================

/**
 * @brief Orchestrates data subscriptions, requests, and distribution
 * 
 * Endpoints:
 * - DataEngine.execute: Handle subscription commands
 * - DataEngine.process: Process incoming market data
 * - DataEngine.request: Handle data pull requests
 * - DataEngine.response: Process data responses
 */
class DataEngine : public Component {
public:
    DataEngine(MessageBus* msgbus, Cache* cache, Clock* clock,
               const DataEngineConfig& config = DataEngineConfig());
    
    ~DataEngine() override = default;
    
    // ========================================================================
    // Component Lifecycle
    // ========================================================================
    
protected:
    void on_initialize() override;
    void on_start() override;
    void on_stop() override;

public:
    // ========================================================================
    // Client Management
    // ========================================================================
    
    /**
     * @brief Register a data client
     */
    void register_client(std::shared_ptr<DataClient> client);
    
    /**
     * @brief Get client by ID
     */
    DataClient* get_client(const ClientId& client_id) const;
    
    /**
     * @brief Get default client for venue
     */
    DataClient* get_client_for_venue(const VenueId& venue) const;
    
    // ========================================================================
    // Message Handlers (registered to MessageBus)
    // ========================================================================
    
    void handle_execute(const std::shared_ptr<Message>& msg);
    void handle_process(const std::shared_ptr<Message>& msg);
    std::shared_ptr<Response> handle_request(const std::shared_ptr<Request>& req);
    void handle_response(const std::shared_ptr<Message>& msg);
    
private:
    DataEngineConfig config_;
    
    // Client routing
    std::unordered_map<ClientId, std::shared_ptr<DataClient>> clients_;
    std::unordered_map<VenueId, ClientId> venue_to_client_;
    
    // TODO: Add subscription tracking
    // TODO: Add bar builders for time-based aggregation
};

// ============================================================================
// LiveDataEngine - Extended version for live trading
// ============================================================================

class LiveDataEngine : public DataEngine {
public:
    LiveDataEngine(MessageBus* msgbus, Cache* cache, Clock* clock,
                   const LiveDataEngineConfig& config);
    
    ~LiveDataEngine() override = default;

private:
    void on_start() override;
    void on_stop() override;

    LiveDataEngineConfig live_config_;
    
    // TODO: Add connection monitoring
    // TODO: Add reconnection logic
};

} // namespace npcTrading
