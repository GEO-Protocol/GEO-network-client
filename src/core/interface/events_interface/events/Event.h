#ifndef GEO_NETWORK_CLIENT_EVENT_H
#define GEO_NETWORK_CLIENT_EVENT_H

#include "../../../common/Constraints.h"
#include "../../../common/memory/MemoryUtils.h"
#include "../../../contractors/addresses/BaseAddress.h"
#include <string>

class Event {

public:
    typedef shared_ptr<Event> Shared;

    enum EventType {
        Topology = 0,
        InitTrustLine = 1,
        CloseTrustLine = 2,
        Payment = 3,
    };

public:
    Event(
        EventType identifier,
        const string &eventInformation);

    const BytesShared data() const;

    const size_t dataSize() const;

    static Event::Shared topologyEvent(
        BaseAddress::Shared nodeAddress,
        vector<BaseAddress::Shared>& nodeNeighbors);

    static Event::Shared initTrustLineEvent(
        BaseAddress::Shared source,
        BaseAddress::Shared destination);

    static Event::Shared closeTrustLineEvent(
        BaseAddress::Shared source,
        BaseAddress::Shared destination);

    static Event::Shared paymentEvent(
        BaseAddress::Shared coordinatorAddress,
        BaseAddress::Shared receiverAddress,
        vector<vector<BaseAddress::Shared>>& paymentPaths);

private:
    EventType mEventIdentifier;
    string mEventInformation;
};


#endif //GEO_NETWORK_CLIENT_EVENT_H