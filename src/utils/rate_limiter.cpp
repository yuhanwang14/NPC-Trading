#include "npcTrading/utils/rate_limiter.hpp"

#include <algorithm>

namespace npcTrading
{
  namespace utils
  {

    namespace
    {
      constexpr double kSecondsPerMinute = 60.0;
    }  // namespace

    TokenBucketRateLimiter::TokenBucketRateLimiter(double tokens_per_minute, double burst_size)
        : tokens_(burst_size),
          burst_size_(burst_size),
          refill_rate_(tokens_per_minute / kSecondsPerMinute),
          last_refill_(std::chrono::steady_clock::now())
    {
    }

    void TokenBucketRateLimiter::refill()
    {
      auto now = std::chrono::steady_clock::now();
      double elapsed_seconds = std::chrono::duration<double>(now - last_refill_).count();
      tokens_ = std::min(burst_size_, tokens_ + elapsed_seconds * refill_rate_);
      last_refill_ = now;
    }

    void TokenBucketRateLimiter::consume(double weight)
    {
      std::unique_lock<std::mutex> lock(mutex_);
      while (true)
      {
        refill();
        if (tokens_ >= weight)
        {
          tokens_ -= weight;
          return;
        }
        // Calculate wait time for sufficient tokens
        double tokens_needed = weight - tokens_;
        double wait_seconds = tokens_needed / refill_rate_;
        cv_.wait_for(lock, std::chrono::duration<double>(wait_seconds));
      }
    }

  }  // namespace utils
}  // namespace npcTrading
