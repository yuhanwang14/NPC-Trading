#pragma once

#include "npcTrading/data_engine.hpp"
#include <memory>
#include <string>

namespace npcTrading {

// Forward declaration of implementation (Pimpl - keeps Boost out of public headers)
class BinanceDataClientImpl;

/**
 * @brief Configuration for BinanceDataClient
 */
struct BinanceDataClientConfig {
    std::string api_key;
    std::string api_secret;
    std::string rest_base_url = "https://api.binance.com";
    std::string ws_base_url = "stream.binance.com";
    int ws_port = 9443;
    bool use_testnet = false;
    int reconnect_delay_ms = 5000;
    int request_timeout_ms = 10000;
};

/**
 * @brief Binance Spot market data client (REST + WebSocket)
 * 
 * Implements DataClient interface to provide:
 * - Real-time market data via WebSocket streams (quotes, trades, bars, order books)
 * - Historical data via REST API (klines, instruments)
 * 
 * Uses Boost.Beast for HTTP/WebSocket communication (hidden behind Pimpl).
 * 
 * WS Streams used:
 * - bookTicker: Best bid/ask quotes
 * - aggTrade: Aggregated trades  
 * - kline: Candlestick bars
 * - depth@100ms: Order book updates
 * 
 * REST Endpoints used:
 * - GET /api/v3/exchangeInfo: Instrument metadata
 * - GET /api/v3/klines: Historical bars
 * - GET /api/v3/depth: Order book snapshot
 */
class BinanceDataClient : public DataClient {
public:
    /**
     * @brief Construct a Binance data client
     * @param client_id Unique identifier for this client
     * @param msgbus MessageBus to emit market data messages
     * @param clock Clock for timestamps
     * @param config Client configuration
     */
    BinanceDataClient(const ClientId& client_id,
                      MessageBus* msgbus,
                      Clock* clock,
                      const BinanceDataClientConfig& config = BinanceDataClientConfig());
    
    ~BinanceDataClient() override;
    
    // Non-copyable, movable
    BinanceDataClient(const BinanceDataClient&) = delete;
    BinanceDataClient& operator=(const BinanceDataClient&) = delete;
    BinanceDataClient(BinanceDataClient&&) noexcept;
    BinanceDataClient& operator=(BinanceDataClient&&) noexcept;
    
    // ========================================================================
    // DataClient Interface - Subscription Methods
    // ========================================================================
    
    void subscribe_bars(const BarType& bar_type) override;
    void unsubscribe_bars(const BarType& bar_type) override;
    void subscribe_quotes(const InstrumentId& instrument_id) override;
    void unsubscribe_quotes(const InstrumentId& instrument_id) override;
    void subscribe_trades(const InstrumentId& instrument_id) override;
    void unsubscribe_trades(const InstrumentId& instrument_id) override;
    void subscribe_order_book(const InstrumentId& instrument_id, int depth = 10) override;
    void unsubscribe_order_book(const InstrumentId& instrument_id) override;
    
    // ========================================================================
    // DataClient Interface - Request Methods (synchronous)
    // ========================================================================
    
    /**
     * @brief Request instrument specification from Binance
     * 
     * Calls GET /api/v3/exchangeInfo and parses PRICE_FILTER, LOT_SIZE filters.
     */
    std::optional<Instrument> request_instrument(const InstrumentId& instrument_id) override;
    
    /**
     * @brief Request historical bars from Binance
     * 
     * Calls GET /api/v3/klines with pagination if needed.
     * BarType.spec() is mapped to Binance interval (e.g., "1m", "5m", "1h").
     */
    std::vector<Bar> request_bars(const BarType& bar_type, Timestamp start, Timestamp end) override;
    
    // ========================================================================
    // DataClient Interface - Connection Management
    // ========================================================================
    
    void connect() override;
    void disconnect() override;
    bool is_connected() const override;

private:
    std::unique_ptr<BinanceDataClientImpl> impl_;
    MessageBus* msgbus_;
    Clock* clock_;
    BinanceDataClientConfig config_;
};

} // namespace npcTrading
