#include "npcTrading/monitoring/metrics_collector.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

namespace npcTrading
{
  namespace monitoring
  {

    // ============================================================================
    // Singleton Instance
    // ============================================================================

    MetricsCollector& MetricsCollector::instance(bool use_shared_memory)
    {
      static MetricsCollector instance(use_shared_memory);
      return instance;
    }

    // ============================================================================
    // Constructor / Destructor
    // ============================================================================

    MetricsCollector::MetricsCollector(bool use_shared_memory)
        : use_shared_memory_(use_shared_memory),
          shm_fd_(-1),
          metrics_(nullptr),
          local_metrics_(nullptr)
    {
      initialize();
    }

    MetricsCollector::~MetricsCollector()
    {
      shutdown();
    }

    // ============================================================================
    // Lifecycle
    // ============================================================================

    void MetricsCollector::initialize()
    {
      if (use_shared_memory_)
      {
        init_shared_memory();
      }
      else
      {
        // Use local memory
        local_metrics_ = std::make_unique<SystemMetrics>();
        metrics_ = local_metrics_.get();
        metrics_->reset();
      }
    }

    void MetricsCollector::shutdown()
    {
      if (use_shared_memory_)
      {
        cleanup_shared_memory();
      }
      else
      {
        local_metrics_.reset();
        metrics_ = nullptr;
      }
    }

    // ============================================================================
    // Shared Memory Management
    // ============================================================================

    void MetricsCollector::init_shared_memory()
    {
      // Create or open shared memory segment
      shm_fd_ = shm_open(DEFAULT_SHM_NAME, O_CREAT | O_RDWR, 0644);
      if (shm_fd_ == -1)
      {
        std::cerr << "[MetricsCollector] Failed to create shared memory: " << strerror(errno) << std::endl;
        // Fallback to local memory
        use_shared_memory_ = false;
        local_metrics_ = std::make_unique<SystemMetrics>();
        metrics_ = local_metrics_.get();
        metrics_->reset();
        return;
      }

      // Set size
      if (ftruncate(shm_fd_, static_cast<off_t>(SHM_SIZE)) == -1)
      {
        std::cerr << "[MetricsCollector] Failed to set shared memory size: " << strerror(errno) << std::endl;
        close(shm_fd_);
        shm_unlink(DEFAULT_SHM_NAME);
        shm_fd_ = -1;
        // Fallback
        use_shared_memory_ = false;
        local_metrics_ = std::make_unique<SystemMetrics>();
        metrics_ = local_metrics_.get();
        metrics_->reset();
        return;
      }

      // Map to memory
      void* ptr = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
      if (ptr == MAP_FAILED)
      {
        std::cerr << "[MetricsCollector] Failed to map shared memory: " << strerror(errno) << std::endl;
        close(shm_fd_);
        shm_unlink(DEFAULT_SHM_NAME);
        shm_fd_ = -1;
        // Fallback
        use_shared_memory_ = false;
        local_metrics_ = std::make_unique<SystemMetrics>();
        metrics_ = local_metrics_.get();
        metrics_->reset();
        return;
      }

      metrics_ = static_cast<SystemMetrics*>(ptr);

      // Initialize metrics (placement new to construct atomics properly)
      new (metrics_) SystemMetrics();
      metrics_->reset();

      std::cout << "[MetricsCollector] Shared memory initialized at " << DEFAULT_SHM_NAME << std::endl;
    }

    void MetricsCollector::cleanup_shared_memory()
    {
      if (metrics_ != nullptr && use_shared_memory_)
      {
        munmap(metrics_, SHM_SIZE);
        metrics_ = nullptr;
      }

      if (shm_fd_ != -1)
      {
        close(shm_fd_);
        // Unlink to remove the shared memory segment
        shm_unlink(DEFAULT_SHM_NAME);
        shm_fd_ = -1;
      }
    }

    // ============================================================================
    // Static Shared Memory Access (for external dashboard)
    // ============================================================================

    SystemMetrics* MetricsCollector::read_shared_memory(const std::string& name)
    {
      int fd = shm_open(name.c_str(), O_RDONLY, 0644);
      if (fd == -1)
      {
        return nullptr;
      }

      void* ptr = mmap(nullptr, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
      close(fd);  // Can close fd after mmap

      if (ptr == MAP_FAILED)
      {
        return nullptr;
      }

      return static_cast<SystemMetrics*>(ptr);
    }

    void MetricsCollector::close_shared_memory(SystemMetrics* ptr)
    {
      if (ptr != nullptr)
      {
        munmap(ptr, SHM_SIZE);
      }
    }

  }  // namespace monitoring
}  // namespace npcTrading
