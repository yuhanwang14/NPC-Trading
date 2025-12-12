#include "npcTrading/clock.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace npcTrading {

// LiveClock implementation
Timestamp LiveClock::now() const {
    return std::chrono::system_clock::now();
}

TimestampNs LiveClock::timestamp_ns() const {
    auto now_time = std::chrono::system_clock::now();
    auto duration = now_time.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

int LiveClock::schedule_callback(Timestamp time, std::function<void()> callback) {
    if (!callback) {
        throw std::invalid_argument("Callback cannot be empty");
    }

    // Create timer record
    auto timer = std::make_shared<Timer>();
    timer->when = time;
    timer->callback = std::move(callback);

    int id;
    {
        std::lock_guard<std::mutex> lock(timers_mutex_);
        id = ++next_timer_id_;
        timers_.emplace(id, timer);
    }

    // Spawn a lightweight worker that sleeps until due time or cancellation
    std::thread([this, timer, id]() {
        std::unique_lock<std::mutex> lk(timer->mutex);
        timer->cv.wait_until(lk, timer->when, [&timer]() { return timer->cancelled.load(); });
        lk.unlock();

        if (!timer->cancelled.load()) {
            try {
                timer->callback();
            } catch (...) {
                // Swallow exceptions from user callbacks to avoid terminating the process
            }
        }

        std::lock_guard<std::mutex> lock(timers_mutex_);
        timers_.erase(id);
    }).detach();

    return id;
}

void LiveClock::cancel_callback(int timer_id) {
    std::shared_ptr<Timer> timer;
    {
        std::lock_guard<std::mutex> lock(timers_mutex_);
        auto it = timers_.find(timer_id);
        if (it == timers_.end()) {
            return;
        }
        timer = it->second;
    }

    {
        std::lock_guard<std::mutex> lk(timer->mutex);
        timer->cancelled.store(true);
    }
    timer->cv.notify_all();
}

// SimulatedClock implementation
SimulatedClock::SimulatedClock(Timestamp start_time)
    : current_time_(start_time) {
}

Timestamp SimulatedClock::now() const {
    return current_time_;
}

TimestampNs SimulatedClock::timestamp_ns() const {
    auto duration = current_time_.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

int SimulatedClock::schedule_callback(Timestamp time, std::function<void()> callback) {
    if (!callback) {
        throw std::invalid_argument("Callback cannot be empty");
    }

    TimerEntry entry;
    entry.when = time;
    entry.callback = std::move(callback);

    int id;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        id = ++next_timer_id_;
        entry.id = id;
        timers_.push(std::move(entry));
    }

    return id;
}

void SimulatedClock::cancel_callback(int timer_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    cancelled_.insert(timer_id);
}

void SimulatedClock::set_time(Timestamp new_time) {
    current_time_ = new_time;
}

void SimulatedClock::process_pending_callbacks() {
    std::vector<std::function<void()>> ready;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!timers_.empty() && timers_.top().when <= current_time_) {
            auto entry = timers_.top();
            timers_.pop();

            if (cancelled_.erase(entry.id) > 0) {
                continue; // Skip cancelled timer
            }

            ready.push_back(std::move(entry.callback));
        }
    }

    for (auto& cb : ready) {
        if (!cb) {
            continue;
        }
        try {
            cb();
        } catch (...) {
            // Ignore callback exceptions to keep simulation running
        }
    }
}

} // namespace npcTrading
