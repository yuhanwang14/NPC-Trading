# NPC Trading System

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.15+-blue.svg)](https://cmake.org/)

An event-driven, high-performance algorithmic trading system for cryptocurrency markets, built with modern C++ and inspired by [Nautilus Trader](https://github.com/nautechsystems/nautilus_trader).

## Overview

NPC Trading is a trading platform featuring:

- **Message-Driven Architecture**: Central MessageBus with pub/sub and req/rep patterns for all inter-component communication
- **Real-Time Market Data**: WebSocket connectivity to Binance streams with quote ticks, trade ticks, OHLCV bars, and L1/L2/L3 order book depth
- **Order Management**: 9 order types (Market, Limit, Stop-Market, Stop-Limit, Take-Profit, Trailing Stop, Limit Maker, Iceberg) with NETTING and HEDGING OMS modes
- **Risk Management**: Pre-trade validation with notional limits, position limits, precision checks, and trading state circuit breakers (ACTIVE / HALTED / REDUCING)
- **Monitoring**: Real-time web dashboard with lock-free shared-memory metrics for cross-process access
- **Strategy Framework**: Event-driven Actor/Strategy base classes with built-in subscription management and order lifecycle callbacks

## Architecture

All components communicate through a central `MessageBus` via registered endpoints:

```
Strategy / ExecAlgorithm
        │ submit_order()
        ▼
   RiskEngine ──deny──▶ OrderDenied
        │ approve
        ▼
 ExecutionEngine
        │ route by venue
        ▼
 ExecutionClient (Binance REST)
        │
        ▼
   OrderEvents ──▶ Cache ──▶ Strategy callbacks
```

| Component | Role | Endpoints |
|-----------|------|-----------|
| **MessageBus** | Event routing (pub/sub + req/rep) | — |
| **Cache** | Shared state: orders, positions, market data, instruments | — |
| **DataEngine** | Market data subscription and distribution | `DataEngine.execute`, `.process`, `.request` |
| **RiskEngine** | Pre-trade validation and trading state | `RiskEngine.execute`, `.process` |
| **ExecutionEngine** | Order routing and position management | `ExecEngine.execute`, `.process` |
| **Monitoring** | Atomic metrics + HTTP dashboard | Shared memory |

For detailed architecture documentation, see [docs/architecture.md](docs/architecture.md).

## Features

### Core Infrastructure
- Central MessageBus with point-to-point, pub/sub, and req/rep messaging
- Component lifecycle management (PRE_INITIALIZED → INITIALIZED → READY → RUNNING → STOPPED → DISPOSED)
- Centralized Cache with write-before-publish consistency, ring-buffer history, and stale-data filtering
- LiveClock with nanosecond timestamps

### Market Data
- Binance WebSocket integration (quotes, trades, bars, order book)
- Order book with L1/L2/L3 depth, incremental delta updates, and analytics (spread, mid-price, VWAP, market impact, imbalance)
- DataEngine with refcounted subscriptions and multi-client venue routing

### Order Types
| Type | Description |
|------|-------------|
| `MARKET` | Immediate execution at best available price |
| `LIMIT` | Execute at specified price or better |
| `STOP_MARKET` | Trigger market order at stop price |
| `STOP_LIMIT` | Trigger limit order at stop price |
| `STOP_LOSS` | Stop-loss (alias for stop-market) |
| `TAKE_PROFIT` | Take-profit market order |
| `TAKE_PROFIT_LIMIT` | Take-profit limit order |
| `LIMIT_MAKER` | Post-only (rejected if would take liquidity) |
| `TRAILING_STOP_MARKET` | Trailing stop based on percentage/amount |

Time-in-force: GTC, IOC, FOK, GTD, GTX (post-only)

### Risk Management
- Notional limits (per-order and per-position)
- Position size limits
- Price and quantity precision validation against instrument specs
- Reduce-only enforcement in REDUCING state
- Short-selling controls
- Trading state circuit breaker (ACTIVE / HALTED / REDUCING)

### Execution
- Abstract `ExecutionClient` interface for exchange adapters
- Binance REST execution client with testnet support
- Order lifecycle events: Submitted → Accepted → Filled / Rejected / Canceled / Denied
- Batch order submission (`submit_order_list`)
- `ExecAlgorithm` base class for complex execution (TWAP, VWAP)

### Monitoring
- Lock-free `SystemMetrics` struct with atomics for cross-process shared memory
- Standalone HTTP dashboard server with real-time JSON API
- Tracks: message throughput, market data counts, order statistics, risk check pass/fail, component states, PnL, uptime

## Getting Started

### Prerequisites

- **CMake** v3.15 or higher
- **C++ Compiler** with C++17 support (GCC 7+, Clang 5+, MSVC 2017+)
- **OpenSSL** (required for Binance integration)
- **Boost** (required for Binance integration: `Boost.Beast`, `Boost.Asio`, `Boost.JSON`)
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

### Creating a Strategy

```cpp
#include "npcTrading/strategy.hpp"
#include "npcTrading/market_data.hpp"

class MyStrategy : public npcTrading::Strategy {
public:
    MyStrategy(const StrategyConfig& config, MessageBus* msgbus, Cache* cache, Clock* clock)
        : Strategy(config, msgbus, cache, clock) {}

    void on_start() override {
        subscribe_bars(BarType("BTCUSDT", "1-MINUTE"));
        subscribe_quotes("BTCUSDT");
    }

    void on_bar(const Bar& bar) override {
        if (!has_position("BTCUSDT")) {
            submit_market_order("BTCUSDT", OrderSide::BUY, Quantity(0.001));
        }
    }

    void on_order_filled(const Order& order, const Fill& fill) override {
        // React to fills
    }
};
```

### Wiring Components

```cpp
#include "npcTrading/message_bus.hpp"
#include "npcTrading/cache.hpp"
#include "npcTrading/clock.hpp"
#include "npcTrading/data_engine.hpp"
#include "npcTrading/risk_engine.hpp"
#include "npcTrading/execution_engine.hpp"
#include "npcTrading/binance/binance_execution_client.hpp"

using namespace npcTrading;

int main() {
    // Infrastructure
    MessageBus msgbus;
    Cache cache;
    LiveClock clock;

    // Engines
    RiskEngine risk_engine(&msgbus, &cache, &clock);
    ExecutionEngine exec_engine(&msgbus, &cache, &clock);
    DataEngine data_engine(&msgbus, &cache, &clock);

    // Binance execution client (config loaded from env vars)
    auto binance = std::make_shared<BinanceExecutionClient>(
        "BINANCE_SPOT", "default", &msgbus, &clock,
        BinanceExecutionClientConfig::from_env());
    exec_engine.register_client(binance);

    // Initialize, start, and run
    risk_engine.initialize(); risk_engine.start();
    exec_engine.initialize(); exec_engine.start();
    data_engine.initialize(); data_engine.start();

    binance->connect();
    msgbus.start();
    msgbus.run();  // Event loop
    return 0;
}
```

### CLI Order Placement

```bash
# Place a market order on Binance testnet
export BINANCE_API_KEY=your_key BINANCE_API_SECRET=your_secret
./bin/Release/npcTrading_binance_place_order \
    --symbol BTCUSDT --side BUY --type MARKET --quantity 0.001

# Limit order with custom time-in-force
./bin/Release/npcTrading_binance_place_order \
    --symbol BTCUSDT --side BUY --type LIMIT --quantity 0.001 --price 50000 --tif IOC
```

### Monitoring Dashboard

```bash
# Start the dashboard (connects via shared memory)
./bin/Release/npcTrading_dashboard --port 8080

# Standalone mode for testing
./bin/Release/npcTrading_dashboard --standalone
```

Open `http://localhost:8080` to view real-time metrics, component states, and connection health.

## Project Structure

```
npcTrading/
├── include/npcTrading/           # Public headers
│   ├── binance/                  # Binance data + execution client
│   ├── monitoring/               # Dashboard server, metrics collector, system metrics
│   ├── utils/                    # Rate limiter
│   ├── cache.hpp                 # Centralized state storage
│   ├── clock.hpp                 # LiveClock / TestClock
│   ├── common.hpp                # Core types, enums, value types
│   ├── component.hpp             # Component lifecycle base class
│   ├── data_engine.hpp           # Market data orchestration
│   ├── execution_engine.hpp      # Order routing + execution client interface
│   ├── market_data.hpp           # Instrument, QuoteTick, TradeTick, Bar, OrderBook
│   ├── message_bus.hpp           # Central event bus
│   ├── model.hpp                 # Order, Fill, Position, Account
│   ├── risk_engine.hpp           # Pre-trade risk validation
│   └── strategy.hpp              # Actor, Strategy, ExecAlgorithm base classes
├── src/                          # Implementation
│   ├── binance/                  # Binance WebSocket + REST clients
│   ├── monitoring/               # Dashboard server + metrics collector
│   └── ...                       # Core engine implementations
├── test/src/                     # Unit tests (GoogleTest)
├── examples/                     # CLI tools (binance_place_order)
├── docs/                         # Architecture documentation
└── cmake/                        # CMake modules (Conan, Vcpkg, compiler warnings)
```

## TODO — Remaining Implementation Work

- [ ] Implement `Position::apply_fill()` with real P&L calculation (currently stub-with-guard)
- [ ] Implement `RiskEngine` validation methods — all checks currently deny when `bypass_risk=false`
- [ ] Implement `ExecutionEngine` position tracking (`handle_fill`, `update_position`)
- [ ] Implement `ExecAlgorithm` child order spawning (`spawn_market_order`, `spawn_limit_order`)
- [ ] Add proper decimal types for `Price`/`Quantity` (replace `double` to avoid float precision issues)
- [ ] Path traversal hardening in `dashboard_server.cpp` file serving
- [ ] Reduce compiler warnings (~16 remaining, mostly unused parameters in stubs)

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Governance](GOVERNANCE.md)
- [Security Policy](SECURITY.md)
- [Roadmap](ROADMAP.md)
- [Maintainers](MAINTAINERS.md)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Nautilus Trader](https://github.com/nautechsystems/nautilus_trader)
- [PandoraTrader](https://github.com/pegasusTrader/PandoraTrader)
- [NexusTrader](https://github.com/Quantweb3-com/NexusTrader)

---

**Disclaimer**: This software is for educational and research purposes. Use at your own risk. Cryptocurrency trading carries substantial risk of loss.
