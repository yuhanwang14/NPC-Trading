# Governance

## Decision-Making

- **Small changes** (docs, bug fixes, typos): Maintainers can approve and merge directly
- **Features**: Open an issue for discussion first, then submit a PR
- **Breaking changes**: Require an RFC-style issue with public discussion before implementation
- **Governance changes**: All maintainers must agree

## Vision & Scope

NPC Trading is an event-driven algorithmic trading system for cryptocurrency markets, built with modern C++17.

### What We Do

- High-performance market data processing (WebSocket, REST)
- Order management with multiple order types and OMS modes
- Pre-trade risk validation
- Strategy framework for user-defined trading logic
- Real-time monitoring and metrics

### What We Don't Do

- Provide trading signals or financial advice
- Support non-crypto asset classes (stocks, forex, etc.)
- Offer hosted/managed trading services
- Guarantee profitability

### We Say No To

Features that:
1. Expand scope beyond cryptocurrency trading infrastructure
2. Add maintenance burden without clear community benefit
3. Introduce security risks (e.g., storing credentials in code)
4. Break backwards compatibility without strong justification

## How to Propose Ideas

1. Open a [GitHub Issue](https://github.com/yuhanwang14/NPC-Trading/issues) to discuss
2. We prioritize based on community need and maintainer capacity
3. PRs without prior discussion may be closed if they add significant scope
