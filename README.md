# NPC Trading System

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.15+-blue.svg)](https://cmake.org/)

An event-driven, high-performance algorithmic trading system for cryptocurrency markets, built with modern C++ and inspired by [Nautilus Trader](https://github.com/nautechsystems/nautilus_trader).

## Overview

NPC Trading is a production-ready trading platform featuring:

- **Message-Driven Architecture**: Unified event bus for all inter-component communication
- **Real-Time Market Data**: WebSocket connectivity to Binance public streams
- **Order Management**: Robust order execution with support for NETTING and HEDGING modes
- **Risk Management**: Pre-trade risk validation, position monitoring, and circuit breakers
- **Monitoring**: Real-time web dashboard for system health and metrics visualization
- **Strategy Framework**: Easy-to-use API for developing event-driven trading strategies

## Architecture

The system is organized around core components communicating via a central MessageBus:

1. **MessageBus**: Central event routing with pub/sub and req/rep patterns
2. **Cache**: Shared state storage for orders, positions, and market data
3. **DataEngine**: Market data subscription and distribution (Binance WebSocket)
4. **RiskEngine**: Pre-trade validation, risk limits, and circuit breakers
5. **ExecutionEngine**: Order routing and execution management (Binance REST)
6. **Monitoring**: System metrics collection and real-time dashboard

For detailed architecture documentation, see [docs/architecture.md](docs/architecture.md).

## Features

### Implemented
- ✅ Core message bus infrastructure
- ✅ Component lifecycle management
- ✅ Centralized cache system
- ✅ Binance WebSocket integration (Market Data)
- ✅ Binance REST integration (Order Execution)
- ✅ Risk Management (Notional limits, Circuit breakers)
- ✅ Real-time Monitoring Dashboard
- ✅ Grid trading strategy example

## Getting Started

### Prerequisites

- **CMake** v3.15 or higher
- **C++ Compiler** with C++17 support (GCC 7+, Clang 5+, MSVC 2017+)
- **OpenSSL** (Required for Binance integration)
- **Boost** (Required for Binance integration, specifically `Boost.Beast` and `Boost.Asio`)
- **GoogleTest** (for unit testing, automatically fetched)

### Building from Source

```bash
# Clone the repository
git clone https://github.com/yuqiannemo/npcTrading.git
cd npcTrading

# Create build directory and configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DnpcTrading_ENABLE_BINANCE=ON

# Build the project
cmake --build . --config Release

# Run tests
ctest -C Release
```

### Installation

```bash
# From the build directory
cmake --build . --target install --config Release
```

## Usage

### Creating a Simple Strategy

```cpp
#include "npcTrading/strategy.hpp"
#include "npcTrading/bar.hpp"

class SimpleMovingAverageStrategy : public Strategy {
public:
    SimpleMovingAverageStrategy(const StrategyConfig& config, MessageBus* msgbus, Cache* cache, Clock* clock)
        : Strategy(config, msgbus, cache, clock), fast_period_(10), slow_period_(30) {
    }
    
    void on_start() override {
        // Subscribe to 1-minute BTCUSDT bars
        subscribe_bars(BarType("BTCUSDT", "1-MINUTE"));
    }
    
    void on_bar(const Bar& bar) override {
        // Calculate moving averages
        double fast_ma = calculate_sma(bars_, fast_period_);
        double slow_ma = calculate_sma(bars_, slow_period_);
        
        // Generate signals
        if (fast_ma > slow_ma && !has_position()) {
            submit_market_order(Side::BUY, Quantity(0.001));
        } else if (fast_ma < slow_ma && has_position()) {
            submit_market_order(Side::SELL, Quantity(0.001));
        }
    }
    
private:
    int fast_period_;
    int slow_period_;
};
```

### Running the System

```cpp
#include "npcTrading/kernel.hpp"
#include "npcTrading/binance/binance_data_client.hpp"
#include "npcTrading/binance/binance_execution_client.hpp"

int main() {
    // Initialize trading kernel
    TradingKernel kernel;
    kernel.initialize_live();
    
    // Configure Binance clients
    BinanceDataConfig data_config;
    // ... config setup ...
    
    BinanceExecConfig exec_config;
    exec_config.api_key = "your_api_key";
    exec_config.api_secret = "your_api_secret";
    
    // Add clients to kernel
    kernel.add_data_client(new BinanceDataClient(data_config));
    kernel.add_exec_client(new BinanceExecClient(exec_config));
    
    // Start system
    kernel.start();
    kernel.run();
    return 0;
}
```

### Monitoring Dashboard

The system includes a standalone monitoring dashboard that connects to the trading engine via shared memory.

```bash
# Start the dashboard (in a separate terminal)
./bin/Release/npcTrading_dashboard --port 8080

# Or run in standalone mode for testing
./bin/Release/npcTrading_dashboard --standalone
```

Open `http://localhost:8080` in your web browser to view real-time metrics, system status, and connection health.

## Project Structure

```
npcTrading/
├── include/npcTrading/         # Public header files
│   ├── monitoring/             # Dashboard and metrics
│   ├── utils/                  # Rate limiters and utilities
│   ├── binance/                # Binance specific headers
│   ├── message_bus.hpp
│   ├── risk_engine.hpp
│   ├── strategy.hpp
│   └── ...
├── src/                        # Implementation files
│   ├── binance/                # Binance integration
│   ├── monitoring/             # Monitoring implementation
│   ├── risk/                   # Risk engine implementation
│   └── ...
├── test/                       # Unit tests
├── docs/                       # Documentation
└── cmake/                      # CMake modules
```

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Nautilus Trader](https://github.com/nautechsystems/nautilus_trader)
- [PandoraTrader](https://github.com/pegasusTrader/PandoraTrader)
- [NexusTrader](https://github.com/Quantweb3-com/NexusTrader)

---

**Disclaimer**: This software is for educational and research purposes. Use at your own risk. Cryptocurrency trading carries substantial risk of loss.
