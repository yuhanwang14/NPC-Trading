set(sources
    src/tmp.cpp
    src/common.cpp
    src/message_bus.cpp
    src/component.cpp
    src/cache.cpp
    src/clock.cpp
    src/data_engine.cpp
    src/execution_engine.cpp
    src/risk_engine.cpp
    src/strategy.cpp
    src/model.cpp
)

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

set(test_sources
  src/cache_test.cpp
  src/message_bus_test.cpp
)
