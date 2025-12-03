#pragma once

#include "common.hpp"
#include "message_bus.hpp"
#include "clock.hpp"
#include <string>
#include <memory>

namespace npcTrading {

// Forward declaration
class Cache;

// ============================================================================
// Component Base Class
// ============================================================================

/**
 * @brief Base class for all engines and actors
 * 
 * Provides:
 * - Lifecycle management with state machine
 * - Access to core infrastructure (MessageBus, Cache, Clock)
 * - Common logging and identification
 */
class Component {
public:
    Component(ComponentId id, 
              MessageBus* msgbus, 
              Cache* cache, 
              Clock* clock);
    
    virtual ~Component() = default;
    
    // ========================================================================
    // Lifecycle Management
    // ========================================================================
    
    /**
     * @brief Pre-initialization phase
     * Override to perform setup before initialization
     */
    virtual void pre_initialize();
    
    /**
     * @brief Initialize the component
     * Override to perform initialization logic
     */
    virtual void initialize();
    
    /**
     * @brief Prepare component for running
     * Override to perform setup before start
     */
    virtual void ready();
    
    /**
     * @brief Start the component
     * Override to begin operation
     */
    virtual void start();
    
    /**
     * @brief Stop the component
     * Override to gracefully shutdown
     */
    virtual void stop();
    
    /**
     * @brief Dispose of component resources
     * Override to clean up resources
     */
    virtual void dispose();
    
    // ========================================================================
    // State Queries
    // ========================================================================
    
    ComponentState state() const { return state_; }
    ComponentId id() const { return id_; }
    bool is_initialized() const { return state_ >= ComponentState::INITIALIZED; }
    bool is_running() const { return state_ == ComponentState::RUNNING; }
    bool is_stopped() const { return state_ == ComponentState::STOPPED; }
    
protected:
    // Core infrastructure (accessible by derived classes)
    ComponentId id_;
    MessageBus* msgbus_;
    Cache* cache_;
    Clock* clock_;
    
    ComponentState state_;
    
    // Helper methods
    void transition_to(ComponentState new_state);
    
    // Logging helpers
    void log_info(const std::string& message);
    void log_warning(const std::string& message);
    void log_error(const std::string& message);
    void log_debug(const std::string& message);
    
private:
    bool validate_state_transition(ComponentState from, ComponentState to) const;
};

} // namespace npcTrading
