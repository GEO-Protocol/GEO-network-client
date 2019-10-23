#include "EventsInterfaceManager.h"

EventsInterfaceManager::EventsInterfaceManager(
    vector<pair<string, SerializedEventType>> filesToEvents,
    vector<pair<string, bool>> filesToBlock,
    Logger &logger):
    mLogger(logger)
{
    for (const auto &fifoFile : filesToBlock) {
        auto fifoFineName = fifoFile.first.c_str();
        auto eventsInterface = make_shared<EventsInterface>(
            fifoFineName,
            fifoFile.second,
            logger);
        for (const auto &fifoAndEvent : filesToEvents) {
            if (fifoAndEvent.first == fifoFile.first) {
                mEventsInterfaces.insert(
                    make_pair(
                        fifoAndEvent.second,
                        eventsInterface));
            }
        }
    }
}

void EventsInterfaceManager::writeEvent(
    Event::Shared event)
{
    auto eventTypeAndInterface = mEventsInterfaces.equal_range(event->type());
    for (auto it = eventTypeAndInterface.first; it != eventTypeAndInterface.second; it++) {
        it->second->writeEvent(event);
    }
}