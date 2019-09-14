#ifndef GEO_NETWORK_CLIENT_EVENTSINTERFACEMANAGER_H
#define GEO_NETWORK_CLIENT_EVENTSINTERFACEMANAGER_H

#include "EventsInterface.h"

class EventsInterfaceManager {

public:
    EventsInterfaceManager(
        vector<pair<string, SerializedEventType>> filesToEvents,
        vector<pair<string, bool>> filesToBlock,
        Logger &logger);

    void writeEvent(
        Event::Shared event);

private:
    multimap<SerializedEventType, shared_ptr<EventsInterface>> mEventsInterfaces;
    Logger &mLogger;
};


#endif //GEO_NETWORK_CLIENT_EVENTSINTERFACEMANAGER_H
