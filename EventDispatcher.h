#pragma once

#include <vector>
#include <unordered_map>
#include <queue>
#include <utility>
#include <type_traits>

#include "Function.h"

/// @brief EventDispatcher is a class that dispatches events to the appropriate event subscribers.
/// @tparam E Enum type of the event, so that the event dispatcher can be used to dispatch events.
// IT MUST BE AN ENUM CLASS.
/// @tparam T Data type passed to the subscriber. The function pointer is derived from this. The function pointer
/// to subscribe to an event is of the form void(*)(T&), It can be any function/member function.
/// Note that the data is passed by reference. In blocking Dispatch() calls, data is mutable (if T isn't const)
/// and the dispatcher can react to the mutated data directly, since everything is passed by reference. In
/// non-blocking queued QueueEvent() this is not the case, since the data is copied for later dispatching and then
/// passed to the subscribers. The dispatcher can't react to the data in this case.
template<typename E, typename T>
class EventDispatcher
{
public:
    /// @brief Enum type of the event.
    using EventEnum_t = std::underlying_type_t<E>;

    /// @brief Function pointer type to subscribe to an event of type T.
    using EventFn = Function<void(const T&)>;

    /// @brief Map of event subscribers.
    using SubscriberMap = std::unordered_map<EventEnum_t, std::vector<EventFn>>;

    /// @brief Queue of events.
    using EventQueue = std::queue<std::pair<E, T>>;

    /// @brief Subscribes to an event of type T.
    /// @param eventType The type of the event.
    /// @param eventFn The function pointer to subscribe.
    void Subscribe(E eventType, const EventFn& eventFn)
    {
        auto& subs = mEventSubscribers[static_cast<EventEnum_t>(eventType)];
        subs.push_back(eventFn);
    }

    /// @brief Unsubscribes from an event of type T. This is a linear search, so it's not very efficient on large
    /// subscriber lists. It's a costlyish operation: O(n). Furthermore Vector::erase() is used so memory
    /// movement is involved.
    /// @param eventType The type of the event.
    /// @param eventFn The function pointer to unsubscribe.
    void Unsubscribe(E eventType, const EventFn& eventFn)
    {
        auto& subs = mEventSubscribers[static_cast<EventEnum_t>(eventType)];
        for (uint32_t i = 0; i < subs.size(); i++)
        {
            if (subs[i] == eventFn)
            {
                subs.erase(subs.begin() + i);
                break;
            }
        }
    }

    /// @brief Dispatches the event to all the subscribers of the event. This is a blocking call.
    /// It Blocks until all the subscribers have finished executing.
    /// @param eventType The type of the event.
    /// @param data The data to be passed to the subscribers. The data is mutable if T is not const.
    void Dispatch(E eventType, T& data)
    {
        const auto subscribers = mEventSubscribers.find(static_cast<EventEnum_t>(eventType));

        if (subscribers == mEventSubscribers.end())
        {
            return; // No subscribers for this event.
        }

        for (auto& eventFn : subscribers->second) { eventFn(data); }
    }

    /// @brief Queue an event to be dispatched later. The queue is processed in DispatchQueuedEvents().
    /// @param eventType The type of the event.
    /// @param data The data to be passed to the subscribers.
    void QueueEvent(E eventType, const T& data) { mEventQueue.push(std::make_pair(eventType, data)); }

    /// @brief Dispatches all the queued events.
    void DispatchQueuedEvents()
    {
        while (!mEventQueue.empty())
        {
            auto& event = mEventQueue.front();
            Dispatch(event.first, event.second);
            mEventQueue.pop();
        }
    }

private:
    SubscriberMap mEventSubscribers;

    EventQueue mEventQueue;
};
