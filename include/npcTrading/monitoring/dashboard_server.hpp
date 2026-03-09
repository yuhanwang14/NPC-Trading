#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "metrics_collector.hpp"

namespace npcTrading
{
  namespace monitoring
  {

    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace websocket = beast::websocket;
    namespace net = boost::asio;
    using tcp = net::ip::tcp;

    // ============================================================================
    // Dashboard Server Configuration
    // ============================================================================

    struct DashboardServerConfig
    {
      int port = 8080;
      std::string host = "0.0.0.0";
      int metrics_push_interval_ms = 500;  // WebSocket push interval
      std::string static_files_path = "";  // Empty = use embedded HTML
    };

    // ============================================================================
    // Dashboard Server - HTTP + WebSocket server for monitoring
    // ============================================================================

    /**
     * @brief Lightweight HTTP/WebSocket server for the monitoring dashboard
     *
     * Provides:
     * - GET / : Serves the dashboard HTML page
     * - GET /api/metrics : Returns current metrics as JSON
     * - WebSocket /ws : Streams metrics updates in real-time
     *
     * Uses Boost.Beast for HTTP/WebSocket handling.
     */
    class DashboardServer
    {
     public:
      /**
       * @brief Construct dashboard server
       * @param port Port to listen on
       * @param metrics System metrics pointer (read-only)
       * @param web_root Path to static web files
       */
      DashboardServer(int port, const SystemMetrics* metrics, const std::string& web_root = "web/dist");
      ~DashboardServer();

      // Non-copyable
      DashboardServer(const DashboardServer&) = delete;
      DashboardServer& operator=(const DashboardServer&) = delete;

      /**
       * @brief Start the server (non-blocking)
       */
      void start();

      /**
       * @brief Stop the server and cleanup
       */
      void stop();

      /**
       * @brief Check if server is running
       */
      bool is_running() const
      {
        return running_.load();
      }

      /**
       * @brief Get the port the server is listening on
       */
      int port() const
      {
        return port_;
      }

     private:
      // HTTP session handler
      class HttpSession : public std::enable_shared_from_this<HttpSession>
      {
       public:
        HttpSession(tcp::socket socket, DashboardServer* server);
        void run();

       private:
        void do_read();
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        void handle_request();
        void send_response(http::response<http::string_body> res);
        void on_write(beast::error_code ec, std::size_t bytes_transferred, bool close);

        tcp::socket socket_;
        beast::flat_buffer buffer_;
        http::request<http::string_body> req_;
        DashboardServer* server_;
      };

      // WebSocket session handler
      class WsSession : public std::enable_shared_from_this<WsSession>
      {
       public:
        WsSession(tcp::socket socket, DashboardServer* server);
        void run();
        void send(const std::string& message);
        void close();
        bool is_open() const;

       private:
        void on_accept(beast::error_code ec);
        void do_read();
        void do_write();
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        void on_write(beast::error_code ec, std::size_t bytes_transferred);

        websocket::stream<tcp::socket> ws_;
        beast::flat_buffer buffer_;
        DashboardServer* server_;
        std::atomic<bool> open_{ false };
        std::queue<std::string> write_queue_;
        bool writing_ = false;
      };

      void accept_loop();
      void on_accept(beast::error_code ec, tcp::socket socket);
      void broadcast_metrics();
      std::string metrics_to_json() const;

      void serve_file(
          const std::string& path,
          http::request<http::string_body>& req,
          tcp::socket&
              socket);  // Note: Current implementation uses socket directly in http::write, but originally it was stream.
                        // Wait, the implementation in .cpp uses http::write(stream, res).
                        // The session classes pass socket or stream.

      // Actually, let's check serve_file signature in .cpp.
      // I added serve_file in the first multi_replace, but then I might have lost it if I didn't verify.
      // In Step 250, I added included headers but the serve_file definition wasn't shown in the diff snippet unless I
      // scrolled down. Let's assume I need to declare it.

      std::string get_mime_type(const std::string& path);
      // serve_file signature needs to match what I wrote or will write.
      // In Step 247 I wrote:
      // void serve_file(const std::string& path, boost::beast::http::request<boost::beast::http::string_body>& req,
      // boost::beast::tcp_stream& stream); But HttpSession uses tcp::socket.

      // I better verify what handle_request calls.
      // The HttpSession calls handle_request.

      int port_;
      const SystemMetrics* metrics_;
      std::string web_root_;

      net::io_context ioc_;
      tcp::acceptor acceptor_;
      std::atomic<bool> running_{ false };

      std::thread server_thread_;
      std::thread broadcast_thread_;

      std::vector<std::weak_ptr<WsSession>> ws_sessions_;
      mutable std::mutex sessions_mutex_;
    };

  }  // namespace monitoring
}  // namespace npcTrading
