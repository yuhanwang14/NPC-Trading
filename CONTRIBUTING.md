# Contributing to NPC Trading

Thank you for your interest in making NPC Trading better! There are several ways you can get involved.

## Finding Issues You Can Help With

* Check [good first issues](https://github.com/yuhanwang14/NPC-Trading/labels/good%20first%20issue) for beginner-friendly tasks
* Look through [help wanted](https://github.com/yuhanwang14/NPC-Trading/labels/help%20wanted) for issues that need community input
* See the [ROADMAP](ROADMAP.md) for planned work

## Development Setup

### Prerequisites

- **CMake** v3.15+
- **C++ Compiler** with C++17 support (GCC 7+, Clang 5+, MSVC 2017+)
- **OpenSSL** (for Binance integration)
- **Boost** (Beast, Asio, JSON)
- **GoogleTest** (fetched automatically)

### First Time Setup

```bash
git clone https://github.com/yuhanwang14/NPC-Trading.git
cd NPC-Trading
mkdir build && cd build
cmake ..
make -j$(nproc)    # Linux
make -j$(sysctl -n hw.ncpu)  # macOS
```

### Running Tests

```bash
cd build && ctest --output-on-failure
```

All 5 tests must pass before submitting a PR.

## How to Contribute

1. **Open an issue first** to discuss the change (for non-trivial work)
2. **Fork the repo** and create a feature branch (`git checkout -b feature/my-change`)
3. **Make your changes** — follow the coding style of surrounding code
4. **Run tests** to verify nothing is broken
5. **Submit a PR** using the [Pull Request template](PULL_REQUEST_TEMPLATE.md)

### DOs

* **DO** create one pull request per issue
* **DO** follow existing coding style (see [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines))
* **DO** include tests whenever possible
* **DO** link the issue you are addressing in the PR
* **DO** write a clear description explaining *why* the change is needed

### DON'Ts

* **DON'T** merge multiple unrelated changes into one PR
* **DON'T** commit directly to `master`
* **DON'T** commit secrets, API keys, or credentials

## Code Formatting

```bash
# Using the CMake target
cmake --build build --target clang-format
```

## Git Workflow

- `master` should always be in a releasable state
- Create feature branches for all changes
- PRs are squash-merged by default
- Include `[skip ci]` in commit messages to skip CI (use sparingly)

## Review Process

After submitting a PR, maintainers will review your code. Multiple iterations may be needed. See [past PRs](https://github.com/yuhanwang14/NPC-Trading/pulls?q=is%3Apr+is%3Aclosed) for examples.

## Related Documents

- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Governance](GOVERNANCE.md)
- [Security Policy](SECURITY.md)
- [Roadmap](ROADMAP.md)
