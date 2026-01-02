/**
 * @file binance_place_order.cpp
 * @brief CLI tool to place a single order on Binance Spot (testnet or mainnet)
 * 
 * Usage:
 *   binance_place_order --symbol BTCUSDT --side BUY --type MARKET --quantity 0.001
 *   binance_place_order --symbol BTCUSDT --side BUY --type LIMIT --quantity 0.001 --price 50000
 * 
 * Environment variables:
 *   BINANCE_API_KEY       - Required
 *   BINANCE_API_SECRET    - Required
 *   BINANCE_TESTNET       - true/false (default: true for safety)
 *   BINANCE_REST_BASE_URL - Optional override
 *   BINANCE_RECV_WINDOW_MS - Optional (default: 5000)
 * 
 * Safety:
 *   When BINANCE_TESTNET=false, you must pass --i-accept-live-trading-risk
 */

#include "npcTrading/binance/binance_execution_client.hpp"
#include "npcTrading/execution_engine.hpp"
#include "npcTrading/risk_engine.hpp"
#include "npcTrading/message_bus.hpp"
#include "npcTrading/cache.hpp"
#include "npcTrading/clock.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

using namespace npcTrading;

// ============================================================================
// Argument parsing helpers
// ============================================================================

static std::string get_arg(int argc, char* argv[], const std::string& name, const std::string& default_value = "") {
    for (int i = 1; i < argc - 1; ++i) {
        if (argv[i] == name || argv[i] == ("--" + name)) {
            return argv[i + 1];
        }
    }
    return default_value;
}

static bool has_flag(int argc, char* argv[], const std::string& name) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == name || argv[i] == ("--" + name)) {
            return true;
        }
    }
    return false;
}

static void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "\nRequired:\n"
              << "  --symbol SYMBOL       Trading pair (e.g., BTCUSDT)\n"
              << "  --side BUY|SELL       Order side\n"
              << "  --type MARKET|LIMIT   Order type\n"
              << "  --quantity QTY        Order quantity\n"
              << "\nOptional:\n"
              << "  --price PRICE         Price for LIMIT orders\n"
              << "  --tif GTC|IOC|FOK     Time in force (default: GTC)\n"
              << "  --client-id ID        Execution client ID (default: BINANCE_SPOT)\n"
              << "  --order-id ID         Custom order ID (default: auto-generated)\n"
              << "\nSafety:\n"
              << "  --i-accept-live-trading-risk   Required when BINANCE_TESTNET=false\n"
              << "\nEnvironment variables:\n"
              << "  BINANCE_API_KEY       API key (required)\n"
              << "  BINANCE_API_SECRET    API secret (required)\n"
              << "  BINANCE_TESTNET       true/false (default: true)\n"
              << std::endl;
}

static std::string generate_order_id() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::ostringstream oss;
    oss << "NPC" << ms;
    return oss.str();
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char* argv[]) {
    // Check for help
    if (has_flag(argc, argv, "-h") || has_flag(argc, argv, "--help") || argc < 2) {
        print_usage(argv[0]);
        return 0;
    }
    
    // Parse arguments
    std::string symbol = get_arg(argc, argv, "symbol");
    std::string side_str = get_arg(argc, argv, "side");
    std::string type_str = get_arg(argc, argv, "type");
    std::string quantity_str = get_arg(argc, argv, "quantity");
    std::string price_str = get_arg(argc, argv, "price", "0");
    std::string tif_str = get_arg(argc, argv, "tif", "GTC");
    std::string client_id = get_arg(argc, argv, "client-id", "BINANCE_SPOT");
    std::string order_id = get_arg(argc, argv, "order-id");
    bool accept_live_risk = has_flag(argc, argv, "--i-accept-live-trading-risk");
    
    // Validate required args
    if (symbol.empty() || side_str.empty() || type_str.empty() || quantity_str.empty()) {
        std::cerr << "Error: Missing required arguments\n";
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse side
    OrderSide side;
    if (side_str == "BUY" || side_str == "buy") {
        side = OrderSide::BUY;
    } else if (side_str == "SELL" || side_str == "sell") {
        side = OrderSide::SELL;
    } else {
        std::cerr << "Error: Invalid side '" << side_str << "' (must be BUY or SELL)\n";
        return 1;
    }
    
    // Parse type
    OrderType type;
    if (type_str == "MARKET" || type_str == "market") {
        type = OrderType::MARKET;
    } else if (type_str == "LIMIT" || type_str == "limit") {
        type = OrderType::LIMIT;
    } else {
        std::cerr << "Error: Invalid type '" << type_str << "' (must be MARKET or LIMIT)\n";
        return 1;
    }
    
    // Parse time in force
    TimeInForce tif = TimeInForce::GTC;
    if (tif_str == "IOC" || tif_str == "ioc") {
        tif = TimeInForce::IOC;
    } else if (tif_str == "FOK" || tif_str == "fok") {
        tif = TimeInForce::FOK;
    }
    
    // Parse quantity and price
    double quantity = std::stod(quantity_str);
    double price = std::stod(price_str);
    
    // Validate LIMIT orders have price
    if (type == OrderType::LIMIT && price <= 0) {
        std::cerr << "Error: LIMIT orders require --price\n";
        return 1;
    }
    
    // Generate order ID if not provided
    if (order_id.empty()) {
        order_id = generate_order_id();
    }
    
    // Load config from environment
    auto config = BinanceExecutionClientConfig::from_env();
    
    // Check credentials
    if (config.api_key.empty() || config.api_secret.empty()) {
        std::cerr << "Error: BINANCE_API_KEY and BINANCE_API_SECRET environment variables required\n";
        return 1;
    }
    
    // Safety check for live trading
    if (!config.use_testnet && !accept_live_risk) {
        std::cerr << "Error: Live trading (BINANCE_TESTNET=false) requires --i-accept-live-trading-risk flag\n";
        return 1;
    }
    
    std::cout << "=== Binance Spot Order Placement ===" << std::endl;
    std::cout << "Environment: " << (config.use_testnet ? "TESTNET" : "MAINNET (LIVE!)") << std::endl;
    std::cout << "Symbol:      " << symbol << std::endl;
    std::cout << "Side:        " << to_string(side) << std::endl;
    std::cout << "Type:        " << to_string(type) << std::endl;
    std::cout << "Quantity:    " << quantity << std::endl;
    if (type == OrderType::LIMIT) {
        std::cout << "Price:       " << price << std::endl;
    }
    std::cout << "Order ID:    " << order_id << std::endl;
    std::cout << "Client ID:   " << client_id << std::endl;
    std::cout << std::endl;
    
    // Create infrastructure
    MessageBusConfig bus_config;
    MessageBus msgbus(bus_config);
    
    CacheConfig cache_config;
    Cache cache(cache_config);
    
    LiveClock clock;
    
    // Create engines
    RiskEngineConfig risk_config;
    risk_config.bypass_risk = true;  // Minimal validation for this CLI
    RiskEngine risk_engine(&msgbus, &cache, &clock, risk_config);
    
    ExecEngineConfig exec_config;
    ExecutionEngine exec_engine(&msgbus, &cache, &clock, exec_config);
    
    // Create Binance execution client
    auto binance_client = std::make_shared<BinanceExecutionClient>(
        client_id,
        "default",
        &msgbus,
        &clock,
        config
    );
    
    // Register client with execution engine
    exec_engine.register_client(binance_client);
    
    // Track order completion
    std::atomic<bool> order_complete{false};
    std::atomic<bool> order_success{false};
    std::string final_status;
    
    // Subscribe to order events
    msgbus.subscribe("OrderEvent", [&](const std::shared_ptr<Message>& msg) {
        if (auto event = std::dynamic_pointer_cast<OrderEvent>(msg)) {
            if (event->order_id() != order_id) return;
            
            std::cout << "[Event] " << msg->type();
            
            if (auto accepted = std::dynamic_pointer_cast<OrderAccepted>(msg)) {
                std::cout << " - Order accepted by exchange" << std::endl;
            } else if (auto rejected = std::dynamic_pointer_cast<OrderRejected>(msg)) {
                std::cout << " - Reason: " << rejected->reason() << std::endl;
                final_status = "REJECTED: " + rejected->reason();
                order_complete = true;
            } else if (auto denied = std::dynamic_pointer_cast<OrderDenied>(msg)) {
                std::cout << " - Reason: " << denied->reason() << std::endl;
                final_status = "DENIED: " + denied->reason();
                order_complete = true;
            } else if (auto filled = std::dynamic_pointer_cast<OrderFilled>(msg)) {
                std::cout << " - Filled " << filled->fill().quantity().as_double() 
                          << " @ " << filled->fill().price().as_double() << std::endl;
                final_status = "FILLED";
                order_success = true;
                order_complete = true;
            } else if (auto canceled = std::dynamic_pointer_cast<OrderCanceled>(msg)) {
                std::cout << " - Order canceled" << std::endl;
                final_status = "CANCELED";
                order_complete = true;
            } else {
                std::cout << std::endl;
            }
        }
    });
    
    // Initialize and start components
    risk_engine.initialize();
    exec_engine.initialize();
    
    msgbus.start();
    
    risk_engine.start();
    exec_engine.start();
    
    // Connect the Binance client
    binance_client->connect();
    
    // Wait for connection
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    if (!binance_client->is_connected()) {
        std::cerr << "Error: Failed to connect to Binance" << std::endl;
        return 1;
    }
    
    // Create and submit order
    auto order = std::make_shared<Order>(
        order_id,
        "CLI",  // strategy_id
        symbol,
        client_id,
        side,
        type,
        Quantity(quantity),
        Price(price),
        tif
    );
    
    auto submit_cmd = std::make_shared<SubmitOrder>(order);
    
    std::cout << "Submitting order..." << std::endl;
    msgbus.send(Endpoints::RISK_ENGINE_EXECUTE, submit_cmd);
    
    // Run message bus and wait for order to complete
    auto start_time = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(30);
    
    while (!order_complete) {
        msgbus.run();
        
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed > timeout) {
            std::cerr << "Error: Timeout waiting for order response" << std::endl;
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Cleanup
    binance_client->disconnect();
    exec_engine.stop();
    risk_engine.stop();
    msgbus.stop();
    
    // Report result
    std::cout << "\n=== Result ===" << std::endl;
    if (order_complete) {
        std::cout << "Status: " << final_status << std::endl;
    } else {
        std::cout << "Status: TIMEOUT (order may still be pending on exchange)" << std::endl;
    }
    
    return order_success ? 0 : 1;
}

