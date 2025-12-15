set(sources
    src/tmp.cpp
    src/common.cpp
    src/message_bus.cpp
    src/component.cpp
    src/cache.cpp
    src/clock.cpp
    src/market_data.cpp
    src/data_engine.cpp
    src/execution_engine.cpp
    src/risk_engine.cpp
    src/strategy.cpp
    src/model.cpp
)

# Conditionally add Binance client sources when enabled
if(${PROJECT_NAME}_ENABLE_BINANCE)
    list(APPEND sources
        src/binance/binance_data_client.cpp
    )
endif()

set(exe_sources
		src/main.cpp
		${sources}
)

set(headers
    include/npcTrading/tmp.hpp
    include/npcTrading/common.hpp
    include/npcTrading/message_bus.hpp
    include/npcTrading/component.hpp
    include/npcTrading/cache.hpp
    include/npcTrading/clock.hpp
    include/npcTrading/market_data.hpp
    include/npcTrading/data_engine.hpp
    include/npcTrading/execution_engine.hpp
    include/npcTrading/risk_engine.hpp
    include/npcTrading/strategy.hpp
    include/npcTrading/model.hpp
)

# Binance header is always available (Pimpl pattern keeps Boost out of public API)
list(APPEND headers
    include/npcTrading/binance/binance_data_client.hpp
)

set(test_sources
  src/cache_test.cpp
  src/message_bus_test.cpp
  src/clock_test.cpp
  src/data_engine_test.cpp
  src/actor_test.cpp
)
