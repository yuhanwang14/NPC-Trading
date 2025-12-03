#include "npcTrading/component.hpp"
#include "npcTrading/cache.hpp"
#include <iostream>

namespace npcTrading {

Component::Component(ComponentId id, MessageBus* msgbus, Cache* cache, Clock* clock)
    : id_(id), msgbus_(msgbus), cache_(cache), clock_(clock),
      state_(ComponentState::PRE_INITIALIZED) {
}

void Component::pre_initialize() {
    transition_to(ComponentState::PRE_INITIALIZED);
}

void Component::initialize() {
    transition_to(ComponentState::INITIALIZED);
}

void Component::ready() {
    transition_to(ComponentState::READY);
}

void Component::start() {
    transition_to(ComponentState::RUNNING);
}

void Component::stop() {
    transition_to(ComponentState::STOPPED);
}

void Component::dispose() {
    transition_to(ComponentState::DISPOSED);
}

void Component::transition_to(ComponentState new_state) {
    if (validate_state_transition(state_, new_state)) {
        state_ = new_state;
        log_info("State transition to " + to_string(new_state));
    } else {
        log_error("Invalid state transition from " + to_string(state_) + 
                  " to " + to_string(new_state));
    }
}

bool Component::validate_state_transition(ComponentState from, ComponentState to) const {
    // TODO: Implement proper state machine validation
    return true;
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
