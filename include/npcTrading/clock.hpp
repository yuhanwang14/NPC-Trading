#pragma once

#include "common.hpp"
#include <chrono>
#include <memory>
#include <functional>

namespace npcTrading {

// ============================================================================
// Clock Interface
// ============================================================================

/**
 * @brief Abstract clock interface for time management
 * 
 * Provides abstraction for both live trading (system time) and 
 * backtesting (simulated time).
 */
class Clock {
public:
    virtual ~Clock() = default;
    
    /**
     * @brief Get current timestamp
     */
    virtual Timestamp now() const = 0;
    
    /**
     * @brief Get current timestamp in nanoseconds since epoch
     */
    virtual TimestampNs timestamp_ns() const = 0;
    
    /**
     * @brief Schedule a callback at a specific time
     * @param time Target timestamp
     * @param callback Function to call
     */
    virtual void schedule_callback(Timestamp time, std::function<void()> callback) = 0;
    
    /**
     * @brief Cancel a scheduled callback
     * @param timer_id ID returned from schedule_callback
     */
    virtual void cancel_callback(int timer_id) = 0;
};

// ============================================================================
// Live Clock - Uses system time
// ============================================================================

class LiveClock : public Clock {
public:
    LiveClock() = default;
    ~LiveClock() override = default;
    
    Timestamp now() const override;
    TimestampNs timestamp_ns() const override;
    
    void schedule_callback(Timestamp time, std::function<void()> callback) override;
    void cancel_callback(int timer_id) override;
    
private:
    // TODO: Implement timer management
    int next_timer_id_ = 0;
};

// ============================================================================
// Simulated Clock - For backtesting
// ============================================================================

class SimulatedClock : public Clock {
public:
    explicit SimulatedClock(Timestamp start_time);
    ~SimulatedClock() override = default;
    
    Timestamp now() const override;
    TimestampNs timestamp_ns() const override;
    
    void schedule_callback(Timestamp time, std::function<void()> callback) override;
    void cancel_callback(int timer_id) override;
    
    /**
     * @brief Advance simulated time
     * @param new_time New current time
     */
    void set_time(Timestamp new_time);
    
    /**
     * @brief Process all callbacks up to current time
     */
    void process_pending_callbacks();
    
private:
    Timestamp current_time_;
    
    // TODO: Implement scheduled callback queue
};

} // namespace npcTrading
