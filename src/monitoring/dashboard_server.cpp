#include "npcTrading/monitoring/dashboard_server.hpp"

#include <boost/json.hpp>  // Keep this for now, as the original metrics_to_json uses it. The instruction implies a full switch to nlohmann::json, but the provided snippet only changes handle_request. I will keep boost/json for metrics_to_json until it's fully removed.
#include <chrono>
#include <filesystem>  // Added
#include <fstream>     // Added
#include <iostream>
#include <sstream>

// Boost.Beast and Asio headers (added from instruction)
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>

// nlohmann/json header removed
// #include <nlohmann/json.hpp>

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace http = beast::http;            // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
namespace net = boost::asio;             // from <boost/asio/io_context.hpp>
using tcp = boost::asio::ip::tcp;        // from <boost/asio/ip/tcp.hpp>
namespace fs = std::filesystem;          // from <filesystem>

namespace npcTrading
{
  namespace monitoring
  {

    // Original `namespace json = boost::json;` is removed as per instruction's implied change to nlohmann::json for API.
    // However, `metrics_to_json` still uses `boost::json::object` and `boost::json::serialize`.
    // I will keep `namespace json = boost::json;` for now to ensure `metrics_to_json` compiles,
    // as the instruction only provided a partial change for `handle_request` and not a full replacement of
    // `metrics_to_json`.
    namespace json = boost::json;

    // ============================================================================
    // ============================================================================
    // DashboardServer Implementation
    // ============================================================================

    // Updated constructor as per instruction
    DashboardServer::DashboardServer(int port, const SystemMetrics* metrics, const std::string& web_root)
        : port_(port),
          metrics_(metrics),
          web_root_(web_root),
          ioc_(1),
          acceptor_(ioc_),
          running_(false)
    {
    }

    DashboardServer::~DashboardServer()
    {
      stop();
    }

    void DashboardServer::start()
    {
      if (running_.load())
      {
        return;
      }

      try
      {
        // Hardcoded host to "0.0.0.0" for now, as `config_.host` is removed.
        // This should ideally be configurable or passed to the constructor.
        auto const address = net::ip::make_address("0.0.0.0");
        auto const port = static_cast<unsigned short>(port_);  // Use port_ member

        tcp::endpoint endpoint{ address, port };

        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(net::socket_base::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen(net::socket_base::max_listen_connections);

        running_.store(true);

        std::cout << "[DashboardServer] Starting on http://" << address.to_string() << ":" << port_
                  << std::endl;  // Updated output

        // Start accepting connections
        accept_loop();

        // Run the server in a separate thread
        server_thread_ = std::thread([this]() { ioc_.run(); });

        // Start broadcast thread for WebSocket updates
        broadcast_thread_ = std::thread(
            [this]()
            {
              while (running_.load())
              {
                // config_.metrics_push_interval_ms is removed, using a default for now.
                // This should ideally be configurable or passed to the constructor.
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // Default to 1 second
                if (running_.load())
                {
                  broadcast_metrics();
                }
              }
            });
      }
      catch (const std::exception& e)
      {
        std::cerr << "[DashboardServer] Failed to start: " << e.what() << std::endl;
        running_.store(false);
      }
    }

    void DashboardServer::stop()
    {
      if (!running_.load())
      {
        return;
      }

      running_.store(false);

      // Close all WebSocket sessions
      {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        for (auto& ws : ws_sessions_)
        {
          if (auto session = ws.lock())
          {
            session->close();
          }
        }
        ws_sessions_.clear();
      }

      // Stop acceptor and io_context
      beast::error_code ec;
      acceptor_.close(ec);
      ioc_.stop();

      if (server_thread_.joinable())
      {
        server_thread_.join();
      }

      if (broadcast_thread_.joinable())
      {
        broadcast_thread_.join();
      }

      std::cout << "[DashboardServer] Stopped" << std::endl;
    }

    void DashboardServer::accept_loop()
    {
      acceptor_.async_accept([this](beast::error_code ec, tcp::socket socket) { on_accept(ec, std::move(socket)); });
    }

    void DashboardServer::on_accept(beast::error_code ec, tcp::socket socket)
    {
      if (ec)
      {
        if (running_.load())
        {
          std::cerr << "[DashboardServer] Accept error: " << ec.message() << std::endl;
        }
        return;
      }

      // Create HTTP session
      std::make_shared<HttpSession>(std::move(socket), this)->run();

      // Continue accepting
      if (running_.load())
      {
        accept_loop();
      }
    }

    void DashboardServer::broadcast_metrics()
    {
      std::string json_data = metrics_to_json();

      std::lock_guard<std::mutex> lock(sessions_mutex_);

      // Remove dead sessions and broadcast to active ones
      auto it = ws_sessions_.begin();
      while (it != ws_sessions_.end())
      {
        if (auto session = it->lock())
        {
          if (session->is_open())
          {
            session->send(json_data);
            ++it;
          }
          else
          {
            it = ws_sessions_.erase(it);
          }
        }
        else
        {
          it = ws_sessions_.erase(it);
        }
      }
    }

    std::string DashboardServer::metrics_to_json() const
    {
      const SystemMetrics* m = metrics_;
      if (!m)
      {
        return "{}";
      }

      json::object obj;

      // MessageBus metrics
      obj["messages_processed"] = m->messages_processed.load();
      obj["queue_size"] = m->queue_size.load();
      obj["messages_per_second"] = m->messages_per_second.load();

      // Market data metrics
      obj["quotes_received"] = m->quotes_received.load();
      obj["trades_received"] = m->trades_received.load();
      obj["bars_received"] = m->bars_received.load();
      obj["order_book_updates"] = m->order_book_updates.load();
      obj["websocket_connected"] = m->websocket_connected.load();

      // Execution metrics
      obj["orders_submitted"] = m->orders_submitted.load();
      obj["orders_accepted"] = m->orders_accepted.load();
      obj["orders_filled"] = m->orders_filled.load();
      obj["orders_rejected"] = m->orders_rejected.load();
      obj["orders_cancelled"] = m->orders_cancelled.load();
      obj["orders_pending"] = m->orders_pending.load();

      // Risk metrics
      obj["risk_checks_passed"] = m->risk_checks_passed.load();
      obj["risk_checks_failed"] = m->risk_checks_failed.load();
      obj["trading_state"] = trading_state_name(m->trading_state.load());

      // Component states
      obj["data_engine_state"] = component_state_name(m->data_engine_state.load());
      obj["exec_engine_state"] = component_state_name(m->exec_engine_state.load());
      obj["risk_engine_state"] = component_state_name(m->risk_engine_state.load());
      obj["strategy_state"] = component_state_name(m->strategy_state.load());

      // System health
      obj["uptime_seconds"] = m->uptime_seconds.load();
      obj["last_heartbeat_ms"] = m->last_heartbeat_ms.load();

      // P&L
      obj["total_unrealized_pnl_cents"] = m->total_unrealized_pnl_cents.load();
      obj["total_realized_pnl_cents"] = m->total_realized_pnl_cents.load();
      obj["open_positions_count"] = m->open_positions_count.load();

      return json::serialize(obj);
    }

    std::string DashboardServer::get_mime_type(const std::string& path)
    {
      auto has_suffix = [&](const std::string& suffix)
      { return path.size() >= suffix.size() && path.compare(path.size() - suffix.size(), suffix.size(), suffix) == 0; };

      if (has_suffix(".html"))
        return "text/html";
      if (has_suffix(".css"))
        return "text/css";
      if (has_suffix(".js"))
        return "application/javascript";
      if (has_suffix(".json"))
        return "application/json";
      if (has_suffix(".png"))
        return "image/png";
      if (has_suffix(".jpg") || has_suffix(".jpeg"))
        return "image/jpeg";
      if (has_suffix(".gif"))
        return "image/gif";
      if (has_suffix(".svg"))
        return "image/svg+xml";
      if (has_suffix(".ico"))
        return "image/x-icon";
      return "text/plain";
    }

    void
    DashboardServer::serve_file(const std::string& target_path, http::request<http::string_body>& req, tcp::socket& socket)
    {
      std::string path = target_path;
      if (path == "/" || path.empty())
        path = "/index.html";

      // Sanitize path to prevent directory traversal
      if (path.find("..") != std::string::npos)
      {
        http::response<http::string_body> res{ http::status::bad_request, req.version() };
        res.set(http::field::server, "DashboardServer");
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = "Illegal path";
        res.prepare_payload();
        http::write(socket, res);
        return;
      }

      std::string full_path = web_root_ + path;
      // Check if file exists
      if (!fs::exists(full_path))
      {
        // If direct file missing, try to serve index.html for SPA (unless it has extension)
        if (path.find('.') == std::string::npos || path == "/index.html")
        {
          full_path = web_root_ + "/index.html";
          if (!fs::exists(full_path))
          {
            http::response<http::string_body> res{ http::status::not_found, req.version() };
            res.set(http::field::server, "DashboardServer");
            res.set(http::field::content_type, "text/plain");
            res.keep_alive(req.keep_alive());
            res.body() = "Not Found";
            res.prepare_payload();
            http::write(socket, res);
            return;
          }
        }
        else
        {
          http::response<http::string_body> res{ http::status::not_found, req.version() };
          res.set(http::field::server, "DashboardServer");
          res.set(http::field::content_type, "text/plain");
          res.keep_alive(req.keep_alive());
          res.body() = "File not found";
          res.prepare_payload();
          http::write(socket, res);
          return;
        }
      }

      beast::error_code ec;
      http::file_body::value_type body;
      body.open(full_path.c_str(), beast::file_mode::scan, ec);

      if (ec)
      {
        http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
        res.set(http::field::server, "DashboardServer");
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = "Failed to open file: " + ec.message();
        res.prepare_payload();
        http::write(socket, res);
        return;
      }

      auto const size = body.size();
      http::response<http::file_body> res{ std::piecewise_construct,
                                           std::make_tuple(std::move(body)),
                                           std::make_tuple(http::status::ok, req.version()) };

      res.set(http::field::server, "DashboardServer");
      res.set(http::field::content_type, get_mime_type(full_path));
      res.content_length(size);
      res.keep_alive(req.keep_alive());

      http::write(socket, res);
    }

    // ============================================================================
    // HttpSession Implementation
    // ============================================================================

    DashboardServer::HttpSession::HttpSession(tcp::socket socket, DashboardServer* server)
        : socket_(std::move(socket)),
          server_(server)
    {
    }

    void DashboardServer::HttpSession::run()
    {
      do_read();
    }

    void DashboardServer::HttpSession::do_read()
    {
      auto self = shared_from_this();

      http::async_read(
          socket_,
          buffer_,
          req_,
          [self](beast::error_code ec, std::size_t bytes_transferred) { self->on_read(ec, bytes_transferred); });
    }

    void DashboardServer::HttpSession::on_read(beast::error_code ec, std::size_t /*bytes_transferred*/)
    {
      if (ec == http::error::end_of_stream)
      {
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        return;
      }

      if (ec)
      {
        return;
      }

      handle_request();
    }

    void DashboardServer::HttpSession::handle_request()
    {
      auto const target = req_.target();

      // Check for WebSocket upgrade
      if (websocket::is_upgrade(req_))
      {
        // Create WebSocket session
        auto ws_session = std::make_shared<WsSession>(std::move(socket_), server_);

        // Register session
        {
          std::lock_guard<std::mutex> lock(server_->sessions_mutex_);
          server_->ws_sessions_.push_back(ws_session);
        }

        ws_session->run();
        return;
      }

      // Handle HTTP requests
      if (target == "/api/metrics")
      {
        http::response<http::string_body> res;
        res.version(req_.version());
        res.keep_alive(req_.keep_alive());
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        res.set(http::field::access_control_allow_origin, "*");
        res.body() = server_->metrics_to_json();
        res.prepare_payload();
        send_response(std::move(res));
        return;
      }

      // Serve static files for all other routes (SPA support)
      server_->serve_file(std::string(target), req_, socket_);
    }

    void DashboardServer::HttpSession::send_response(http::response<http::string_body> res)
    {
      auto self = shared_from_this();
      auto sp = std::make_shared<http::response<http::string_body>>(std::move(res));

      http::async_write(
          socket_,
          *sp,
          [self, sp](beast::error_code ec, std::size_t bytes_transferred)
          { self->on_write(ec, bytes_transferred, sp->need_eof()); });
    }

    void DashboardServer::HttpSession::on_write(beast::error_code ec, std::size_t /*bytes_transferred*/, bool close)
    {
      if (ec)
      {
        return;
      }

      if (close)
      {
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        return;
      }

      // Read another request
      req_ = {};
      buffer_.consume(buffer_.size());
      do_read();
    }

    // ============================================================================
    // WsSession Implementation
    // ============================================================================

    DashboardServer::WsSession::WsSession(tcp::socket socket, DashboardServer* server)
        : ws_(std::move(socket)),
          server_(server)
    {
    }

    void DashboardServer::WsSession::run()
    {
      auto self = shared_from_this();

      ws_.async_accept([self](beast::error_code ec) { self->on_accept(ec); });
    }

    void DashboardServer::WsSession::on_accept(beast::error_code ec)
    {
      if (ec)
      {
        return;
      }

      open_.store(true);

      // Send initial metrics
      send(server_->metrics_to_json());

      // Start reading (to detect disconnect)
      do_read();
    }

    void DashboardServer::WsSession::do_read()
    {
      auto self = shared_from_this();

      ws_.async_read(
          buffer_, [self](beast::error_code ec, std::size_t bytes_transferred) { self->on_read(ec, bytes_transferred); });
    }

    void DashboardServer::WsSession::on_read(beast::error_code ec, std::size_t /*bytes_transferred*/)
    {
      if (ec)
      {
        open_.store(false);
        return;
      }

      buffer_.consume(buffer_.size());
      do_read();
    }

    void DashboardServer::WsSession::send(const std::string& message)
    {
      if (!open_.load()) { return; }
      write_queue_.push(message);
      if (!writing_) {
        do_write();
      }
    }

    void DashboardServer::WsSession::do_write()
    {
      if (write_queue_.empty()) {
        writing_ = false;
        return;
      }
      writing_ = true;
      auto self = shared_from_this();
      auto& front = write_queue_.front();
      ws_.text(true);
      ws_.async_write(
          net::buffer(front),
          [self](beast::error_code ec, std::size_t /*bytes_transferred*/) {
            self->write_queue_.pop();
            if (ec) {
              self->open_.store(false);
              return;
            }
            self->do_write();
          });
    }

    void DashboardServer::WsSession::on_write(beast::error_code ec, std::size_t /*bytes_transferred*/)
    {
      if (ec)
      {
        open_.store(false);
      }
    }

    void DashboardServer::WsSession::close()
    {
      if (open_.load())
      {
        open_.store(false);
        beast::error_code ec;
        ws_.close(websocket::close_code::normal, ec);
      }
    }

    bool DashboardServer::WsSession::is_open() const
    {
      return open_.load();
    }

  }  // namespace monitoring
}  // namespace npcTrading
