#ifndef GEO_NETWORK_CLIENT_EVENTSINTERFACE_H
#define GEO_NETWORK_CLIENT_EVENTSINTERFACE_H

#include "../../BaseFIFOInterface.h"
#include "../events/Event.h"
#include "../../../logger/Logger.h"

class EventsInterface : public BaseFIFOInterface {

public:
    explicit EventsInterface(
        Logger &logger);

    ~EventsInterface();

    void writeEvent(
        Event::Shared event);

private:
    virtual const char* FIFOName() const;

public:
    static const constexpr char *kFIFOName = "events.fifo";
    static const constexpr unsigned int kPermissionsMask = 0755;

private:
    Logger &mLogger;
};


#endif //GEO_NETWORK_CLIENT_EVENTSINTERFACE_H
