/**
 * @file dashboard_main.cpp
 * @brief Standalone monitoring dashboard executable
 *
 * Run with: ./npcTrading_dashboard [--port 8080]
 *
 * The dashboard connects to shared memory written by the trading system
 * and displays real-time metrics via a web interface.
 */

#include <signal.h>

#include <atomic>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "npcTrading/monitoring/dashboard_server.hpp"
#include "npcTrading/monitoring/metrics_collector.hpp"

using namespace npcTrading::monitoring;

// Global shutdown flag
std::atomic<bool> g_running(true);
DashboardServer* g_server = nullptr;

void signal_handler(int /*sig*/)
{
  std::cout << "\nShutting down dashboard..." << std::endl;
  g_running = false;
  if (g_server)
  {
    g_server->stop();
  }
}

void print_usage(const char* program)
{
  std::cout << "Usage: " << program << " [OPTIONS]\n"
            << "\nOptions:\n"
            << "  --port PORT       Port to listen on (default: 8080)\n"
            << "  --host HOST       Host to bind to (default: 0.0.0.0)\n"
            << "  --interval MS     Metrics push interval in ms (default: 500)\n"
            << "  --standalone      Run with local metrics (for testing without trading system)\n"
            << "  --web-root PATH   Path to web assets (default: web/dist or relative to executable)\n"
            << "  --help            Show this help message\n"
            << "\nExample:\n"
            << "  " << program << " --port 8080\n"
            << "  Open http://localhost:8080 in your browser\n"
            << std::endl;
}

int main(int argc, char* argv[])
{
  int port = 8080;
  std::string host = "0.0.0.0";
  int metrics_push_interval_ms = 500;
  std::string web_root = "web/dist";
  bool standalone = false;

  // Parse command line arguments
  for (int i = 1; i < argc; ++i)
  {
    if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
    {
      port = std::stoi(argv[++i]);
    }
    else if (strcmp(argv[i], "--host") == 0 && i + 1 < argc)
    {
      host = argv[++i];
    }
    else if (strcmp(argv[i], "--interval") == 0 && i + 1 < argc)
    {
      metrics_push_interval_ms = std::stoi(argv[++i]);
    }
    else if (strcmp(argv[i], "--web-root") == 0 && i + 1 < argc)
    {
      web_root = argv[++i];
    }
    else if (strcmp(argv[i], "--standalone") == 0)
    {
      standalone = true;
    }
    else if (strcmp(argv[i], "--help") == 0)
    {
      print_usage(argv[0]);
      return 0;
    }
    else
    {
      std::cerr << "Unknown option: " << argv[i] << std::endl;
      print_usage(argv[0]);
      return 1;
    }
  }

  // Set up signal handlers
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  SystemMetrics* metrics = nullptr;

  if (standalone)
  {
    // Use local metrics collector for testing
    std::cout << "[Dashboard] Running in standalone mode with local metrics\n";
    auto& collector = MetricsCollector::instance(false);
    collector.mark_started();
    metrics = collector.metrics();

    // Simulate some initial values for testing
    collector.set_websocket_connected(true);
    collector.set_trading_state(2);      // ACTIVE
    collector.set_data_engine_state(3);  // RUNNING
    collector.set_exec_engine_state(3);  // RUNNING
    collector.set_risk_engine_state(3);  // RUNNING
    collector.set_strategy_state(3);     // RUNNING
  }
  else
  {
    // Open shared memory from trading system
    std::cout << "[Dashboard] Connecting to trading system via shared memory...\n";
    metrics = MetricsCollector::read_shared_memory();

    if (!metrics)
    {
      std::cerr << "[Dashboard] Warning: Shared memory not available. "
                << "Is the trading system running?\n"
                << "Use --standalone flag to run without trading system.\n";

      // Fall back to local metrics
      auto& collector = MetricsCollector::instance(false);
      collector.mark_started();
      metrics = collector.metrics();
    }
    else
    {
      std::cout << "[Dashboard] Connected to trading system metrics\n";
    }
  }

  // Create and start server
  DashboardServer server(port, metrics, web_root);
  g_server = &server;

  std::cout << "\n╔════════════════════════════════════════════════════════════╗\n"
            << "║   npcTrading Monitoring Dashboard                          ║\n"
            << "╠════════════════════════════════════════════════════════════╣\n"
            << "║   Open in browser: http://" << host << ":" << port;

  // Pad the URL to align the box
  std::string url = "http://" + host + ":" + std::to_string(port);
  for (size_t j = url.length(); j < 35; ++j)
  {
    std::cout << " ";
  }
  std::cout << "║\n"
            << "║   Press Ctrl+C to stop                                     ║\n"
            << "╚════════════════════════════════════════════════════════════╝\n"
            << std::endl;

  server.start();

  // Keep main thread alive while server runs
  while (server.is_running())
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Update heartbeat if in standalone mode
    if (standalone)
    {
      MetricsCollector::instance(false).heartbeat();

      // Simulate some activity for demo
      static uint64_t counter = 0;
      counter++;
      auto& collector = MetricsCollector::instance(false);
      if (counter % 2 == 0)
        collector.increment_quotes_received();
      if (counter % 3 == 0)
        collector.increment_trades_received();
      if (counter % 5 == 0)
        collector.increment_messages_processed();
    }
  }

  // Cleanup
  if (!standalone && metrics)
  {
    MetricsCollector::close_shared_memory(metrics);
  }

  std::cout << "[Dashboard] Goodbye!\n";
  return 0;
}
