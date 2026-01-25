#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace npcTrading {
namespace utils {

/// Thread-safe Token Bucket Rate Limiter.
/// Allows bursting up to `burst_size` tokens, then refills at `tokens_per_minute` rate.
class TokenBucketRateLimiter {
public:
    /// @param tokens_per_minute Rate at which tokens are added to the bucket.
    /// @param burst_size Maximum number of tokens the bucket can hold.
    TokenBucketRateLimiter(double tokens_per_minute, double burst_size);

    /// Consume `weight` tokens, blocking if insufficient tokens are available.
    /// @param weight Number of tokens to consume (default 1.0).
    void consume(double weight = 1.0);

private:
    void refill();

    std::mutex mutex_;
    std::condition_variable cv_;
    double tokens_;
    double burst_size_;
    double refill_rate_;  // tokens per second
    std::chrono::steady_clock::time_point last_refill_;
};

}  // namespace utils
}  // namespace npcTrading
