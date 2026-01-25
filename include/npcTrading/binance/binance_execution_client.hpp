#pragma once

#include <memory>
#include <string>

#include "npcTrading/clock.hpp"
#include "npcTrading/execution_engine.hpp"
#include "npcTrading/message_bus.hpp"

namespace npcTrading
{

  // Forward declaration of implementation (Pimpl - keeps Boost out of public headers)
  class BinanceExecutionClientImpl;

  /**
   * @brief Configuration for BinanceExecutionClient
   *
   * Credentials are read from environment variables by default:
   *   BINANCE_API_KEY, BINANCE_API_SECRET, BINANCE_TESTNET
   */
  struct BinanceExecutionClientConfig
  {
    std::string api_key;
    std::string api_secret;
    std::string rest_base_url;  // Set automatically based on testnet flag if empty
    std::string ws_base_url;    // For user data stream
    int ws_port = 9443;
    bool use_testnet = true;  // Default to testnet for safety
    int recv_window_ms = 5000;
    int reconnect_delay_ms = 5000;
    int request_timeout_ms = 10000;

    /**
     * @brief Load configuration from environment variables
     *
     * Reads:
     *   BINANCE_API_KEY
     *   BINANCE_API_SECRET
     *   BINANCE_TESTNET (true/false, default true)
     *   BINANCE_REST_BASE_URL (optional override)
     *   BINANCE_RECV_WINDOW_MS (optional, default 5000)
     */
    static BinanceExecutionClientConfig from_env();
  };

  /**
   * @brief Binance Spot execution client (REST order placement + WS user data stream)
   *
   * Implements ExecutionClient interface to provide:
   * - Order submission via signed REST POST /api/v3/order
   * - Order cancellation via signed DELETE /api/v3/order
   * - Real-time execution reports via User Data Stream WebSocket
   *
   * REST Endpoints used:
   * - POST /api/v3/order: Place new order
   * - DELETE /api/v3/order: Cancel order
   * - POST /api/v3/userDataStream: Create listen key for WS
   * - PUT /api/v3/userDataStream: Keep-alive listen key
   *
   * WS Streams used:
   * - User Data Stream: executionReport events
   */
  class BinanceExecutionClient : public ExecutionClient
  {
   public:
    /**
     * @brief Construct a Binance execution client
     * @param client_id Unique identifier for this client (used for order routing)
     * @param account_id Account identifier
     * @param msgbus MessageBus to emit order events
     * @param clock Clock for timestamps
     * @param config Client configuration
     */
    BinanceExecutionClient(
        const ClientId& client_id,
        const AccountId& account_id,
        MessageBus* msgbus,
        Clock* clock,
        const BinanceExecutionClientConfig& config);

    ~BinanceExecutionClient() override;

    // Non-copyable, movable
    BinanceExecutionClient(const BinanceExecutionClient&) = delete;
    BinanceExecutionClient& operator=(const BinanceExecutionClient&) = delete;
    BinanceExecutionClient(BinanceExecutionClient&&) noexcept;
    BinanceExecutionClient& operator=(BinanceExecutionClient&&) noexcept;

    // ========================================================================
    // ExecutionClient Interface - Order Operations
    // ========================================================================

    /**
     * @brief Submit a new order to Binance
     *
     * Sends OrderSubmitted immediately, then calls POST /api/v3/order.
     * On response, sends OrderAccepted or OrderRejected.
     * If order is filled immediately (MARKET), sends OrderFilled.
     */
    void submit_order(Order* order) override;

    /**
     * @brief Modify an existing order (cancel + replace)
     *
     * Binance Spot doesn't support native modify, so this cancels and re-submits.
     */
    void modify_order(Order* order, Quantity new_quantity, Price new_price) override;

    /**
     * @brief Cancel an existing order
     */
    void cancel_order(Order* order) override;

    /**
     * @brief Query order status from Binance
     */
    void query_order(const OrderId& order_id) override;

    // ========================================================================
    // ExecutionClient Interface - Connection Management
    // ========================================================================

    /**
     * @brief Connect to Binance (starts user data stream)
     */
    void connect() override;

    /**
     * @brief Disconnect from Binance
     */
    void disconnect() override;

    /**
     * @brief Check if connected
     */
    bool is_connected() const override;

    // ========================================================================
    // Robustness & Rate Limiting
    // ========================================================================

    /**
     * @brief Attempt to cancel all open orders (safety mechanism)
     *
     * Called automatically on disconnect for safety. Can also be called manually.
     */
    void sync_open_orders();

   private:
    std::unique_ptr<BinanceExecutionClientImpl> impl_;
    MessageBus* msgbus_;
    Clock* clock_;
    BinanceExecutionClientConfig config_;
  };

}  // namespace npcTrading
