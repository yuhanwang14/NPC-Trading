# NPC Trading System

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.15+-blue.svg)](https://cmake.org/)

An event-driven, high-performance algorithmic trading system for cryptocurrency markets, built with modern C++ and inspired by [Nautilus Trader](https://github.com/nautechsystems/nautilus_trader).

## Overview

NPC Trading is a production-ready trading platform featuring:

- **Message-Driven Architecture**: Unified event bus for all inter-component communication
- **Real-Time Market Data**: WebSocket connectivity to cryptocurrency exchanges
- **Order Management**: Complete OMS with support for NETTING and HEDGING modes
- **Risk Management**: Pre-trade risk validation and position monitoring
- **Strategy Framework**: Easy-to-use API for developing trading strategies
- **Backtesting**: Historical data replay for strategy validation (planned)

## Architecture

The system is organized around five core components:

1. **MessageBus**: Central event routing with pub/sub and req/rep patterns
2. **Cache**: Shared state storage for orders, positions, and market data
3. **DataEngine**: Market data subscription and distribution
4. **RiskEngine**: Pre-trade validation and risk limits
5. **ExecutionEngine**: Order routing and execution management

For detailed architecture documentation, see [docs/architecture.md](docs/architecture.md).

## Features

### Current (MVP Phase)

- ✅ Core message bus infrastructure
- ✅ Component lifecycle management
- ✅ Centralized cache system
- 🚧 Binance WebSocket integration (in progress)
- 🚧 Order execution and management (in progress)
- 🚧 Grid trading strategy (in progress)

### Planned

- ⏳ Backtesting framework
- ⏳ Advanced risk metrics (VaR, drawdown limits)
- ⏳ Execution algorithms (TWAP, VWAP)
- ⏳ Multi-exchange support
- ⏳ Performance analytics

## Getting Started

### Prerequisites

- **CMake** v3.15 or higher
- **C++ Compiler** with C++17 support (GCC 7+, Clang 5+, MSVC 2017+)
- **GoogleTest** (for unit testing)
- **Doxygen** (optional, for documentation generation)

### Building from Source

```bash
# Clone the repository
git clone https://github.com/yuqiannemo/npcTrading.git
cd npcTrading

# Create build directory and configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build . --config Release

# Run tests
ctest -C Release
```

### Installation

```bash
# From the build directory
cmake --build . --target install --config Release

# Or specify custom installation directory
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build . --target install --config Release
```

## Usage

### Creating a Simple Strategy

```cpp
#include "npcTrading/strategy.hpp"
#include "npcTrading/bar.hpp"

class SimpleMovingAverageStrategy : public Strategy {
public:
    SimpleMovingAverageStrategy(const StrategyConfig& config)
        : Strategy(config), fast_period_(10), slow_period_(30) {
        subscribe_bars(BarType("BTCUSDT", "1-MINUTE"));
    }
    
    void on_start() override {
        log_info("Strategy started");
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

### Running the Strategy

```cpp
#include "npcTrading/kernel.hpp"
#include "my_strategy.hpp"

int main() {
    // Initialize trading kernel
    TradingKernel kernel;
    kernel.initialize_live();
    
    // Configure Binance data client
    BinanceDataConfig data_config;
    data_config.api_key = "your_api_key";
    data_config.api_secret = "your_api_secret";
    
    // Configure Binance execution client
    BinanceExecConfig exec_config;
    exec_config.api_key = "your_api_key";
    exec_config.api_secret = "your_api_secret";
    
    // Add clients to kernel
    kernel.add_data_client(new BinanceDataClient(data_config));
    kernel.add_exec_client(new BinanceExecClient(exec_config));
    
    // Add strategy
    StrategyConfig strategy_config;
    strategy_config.strategy_id = StrategyId("SMA_001");
    kernel.add_strategy(new SimpleMovingAverageStrategy(strategy_config));
    
    // Start the system
    kernel.start();
    
    // Run until stopped
    kernel.run();
    
    return 0;
}
```

## Development Roadmap

### Phase 1: Core Infrastructure (Target: 2024-12-05)
- [x] MessageBus implementation
- [x] Component base class
- [x] Cache system
- [x] Clock abstraction

### Phase 2: Binance Integration (Target: 2024-12-11)
- [ ] DataEngine implementation
- [ ] BinanceDataClient (WebSocket)
- [ ] Market data objects (QuoteTick, TradeTick, Bar)
- [ ] First successful order execution

### Phase 3: Execution System (Target: 2024-12-25)
- [ ] RiskEngine validation
- [ ] ExecutionEngine routing
- [ ] BinanceExecutionClient (REST)
- [ ] Grid trading strategy
- [ ] Order state management

### Future Phases
- [ ] Backtesting framework
- [ ] Batch data processing
- [ ] Advanced risk engine
- [ ] Performance analytics

## Project Structure

```
npcTrading/
├── include/npcTrading/    # Public header files
│   ├── message_bus.hpp
│   ├── component.hpp
│   ├── cache.hpp
│   ├── data_engine.hpp
│   ├── execution_engine.hpp
│   ├── risk_engine.hpp
│   └── strategy.hpp
├── src/                   # Implementation files
├── test/                  # Unit tests
├── docs/                  # Documentation
│   └── architecture.md    # Detailed architecture guide
├── examples/              # Example strategies (planned)
└── cmake/                 # CMake modules
```

## Generating Documentation

The project uses Doxygen for API documentation:

```bash
# Configure with Doxygen enabled
cd build
cmake .. -DnpcTrading_ENABLE_DOXYGEN=1

# Generate documentation
cmake --build . --target doxygen-docs

# Documentation will be in docs/html/index.html
```

## Testing

```bash
# Run all tests
cd build
ctest -VV

# Run specific test
./test/tmp_test_Tests

# Run with code coverage (if enabled)
cmake .. -DnpcTrading_ENABLE_CODE_COVERAGE=1
cmake --build . --target coverage
```

## Configuration Options

Available CMake options:

```bash
# Enable/disable features
-DnpcTrading_ENABLE_UNIT_TESTING=ON      # Build tests (default: ON)
-DnpcTrading_ENABLE_DOXYGEN=ON           # Generate docs (default: OFF)
-DnpcTrading_ENABLE_CODE_COVERAGE=ON     # Code coverage (default: OFF)
-DnpcTrading_USE_CONAN=ON                # Use Conan package manager
-DnpcTrading_USE_VCPKG=ON                # Use vcpkg package manager

# Build options
-DCMAKE_BUILD_TYPE=Release               # Release or Debug
-DCMAKE_INSTALL_PREFIX=/path/to/install  # Installation directory
```

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Team

- **Wang Yuhan** - Core engine development
- **Yu Qian** - Architecture and integration
- **Lin Siyu** - Strategy framework and testing

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Nautilus Trader](https://github.com/nautechsystems/nautilus_trader) - Architectural inspiration
- [PandoraTrader](https://github.com/pegasusTrader/PandoraTrader) - Design patterns
- [NexusTrader](https://github.com/Quantweb3-com/NexusTrader) - Implementation reference

## Contact

For questions or suggestions, please open an issue on GitHub.

---

**Disclaimer**: This software is for educational and research purposes. Use at your own risk. Cryptocurrency trading carries substantial risk of loss.
