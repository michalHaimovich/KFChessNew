#pragma once
#include <unordered_map>
#include <vector>
#include <functional>
#include <typeindex>
#include <memory>
#include "events.hpp"

class EventBus {
private:
    using EventCallback = std::function<void(const Event*)>;
    // Maps an event type to a list of callback functions
    std::unordered_map<std::type_index, std::vector<EventCallback>> subscribers;

public:
    // Subscribe to a specific event type
    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> callback) {
        auto wrapper = [callback](const Event* baseEvent) {
            callback(*static_cast<const EventType*>(baseEvent));
        };
        subscribers[typeid(EventType)].push_back(wrapper);
    }

    // Publish an event to all interested subscribers
    template<typename EventType>
    void publish(const EventType& event) {
        auto it = subscribers.find(typeid(EventType));
        if (it != subscribers.end()) {
            for (const auto& callback : it->second) {
                callback(&event);
            }
        }
    }
};