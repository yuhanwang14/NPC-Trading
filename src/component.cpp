#include "npcTrading/component.hpp"
#include "npcTrading/cache.hpp"
#include <iostream>
#include <utility>

namespace npcTrading {

Component::Component(ComponentId id, MessageBus* msgbus, Cache* cache, Clock* clock)
    : id_(std::move(id)), msgbus_(msgbus), cache_(cache), clock_(clock),
      state_(ComponentState::PRE_INITIALIZED) {
    if (id_.empty()) {
        throw std::invalid_argument("Component id cannot be empty");
    }
    if (!msgbus_) {
        throw std::invalid_argument("MessageBus pointer cannot be null");
    }
    if (!cache_) {
        throw std::invalid_argument("Cache pointer cannot be null");
    }
    if (!clock_) {
        throw std::invalid_argument("Clock pointer cannot be null");
    }
}

void Component::pre_initialize() {
    transition_to(ComponentState::PRE_INITIALIZED);
    on_pre_initialize();
}

void Component::initialize() {
    transition_to(ComponentState::INITIALIZED);
    on_initialize();
}

void Component::ready() {
    transition_to(ComponentState::READY);
    on_ready();
}

void Component::start() {
    transition_to(ComponentState::RUNNING);
    on_start();
}

void Component::stop() {
    transition_to(ComponentState::STOPPED);
    on_stop();
}

void Component::dispose() {
    transition_to(ComponentState::DISPOSED);
    on_dispose();
}

void Component::transition_to(ComponentState new_state) {
    std::lock_guard<std::mutex> lock(state_mutex_);

    ComponentState current = state_.load();

    if (current == new_state) {
        return; // idempotent
    }

    if (validate_state_transition(current, new_state)) {
        state_.store(new_state);
        log_info("State transition " + to_string(current) + " -> " + to_string(new_state));
    } else {
        std::string msg = "Invalid state transition from " + to_string(current) +
                          " to " + to_string(new_state);
        log_error(msg);
        throw std::logic_error(msg);
    }
}

bool Component::validate_state_transition(ComponentState from, ComponentState to) const {
    switch (from) {
        case ComponentState::PRE_INITIALIZED:
            return to == ComponentState::INITIALIZED;
        case ComponentState::INITIALIZED:
            return to == ComponentState::READY || to == ComponentState::RUNNING;
        case ComponentState::READY:
            return to == ComponentState::RUNNING || to == ComponentState::STOPPED;
        case ComponentState::RUNNING:
            return to == ComponentState::STOPPED;
        case ComponentState::STOPPED:
            return to == ComponentState::DISPOSED || to == ComponentState::STOPPED;
        case ComponentState::DISPOSED:
            return to == ComponentState::DISPOSED;
        default:
            return false;
    }
}

void Component::log_info(const std::string& message) {
    std::cout << "[INFO] " << id_ << ": " << message << std::endl;
}

void Component::log_warning(const std::string& message) {
    std::cout << "[WARNING] " << id_ << ": " << message << std::endl;
}

void Component::log_error(const std::string& message) {
    std::cerr << "[ERROR] " << id_ << ": " << message << std::endl;
}

void Component::log_debug(const std::string& message) {
    std::cout << "[DEBUG] " << id_ << ": " << message << std::endl;
}

} // namespace npcTrading
