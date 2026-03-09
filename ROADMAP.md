# Roadmap

## Short Term (Next 1–2 Releases)

- [ ] Implement `Position::apply_fill()` with real P&L calculation
- [ ] Implement `RiskEngine` validation methods (price precision, quantity limits, notional checks)
- [ ] Implement `ExecutionEngine` position tracking (`handle_fill`, `update_position`)
- [ ] Implement `ExecAlgorithm` child order spawning
- [ ] Reduce compiler warnings (~16 remaining)

## Medium Term (3–6 Months)

- [ ] Add proper decimal types for `Price`/`Quantity` (replace `double`)
- [ ] Path traversal hardening in dashboard file serving
- [ ] Backtesting engine with `SimulatedClock` integration
- [ ] Multi-exchange support (beyond Binance)
- [ ] Persistent order/position storage

## Long Term (Vision)

- Full backtesting framework with historical data replay
- Plugin system for custom indicators and signal generators
- Performance benchmarking suite
- WebSocket-based live strategy monitoring

## How to Propose Ideas

Open a [GitHub Issue](https://github.com/yuhanwang14/NPC-Trading/issues) to discuss. We prioritize based on community need and maintainer capacity.
