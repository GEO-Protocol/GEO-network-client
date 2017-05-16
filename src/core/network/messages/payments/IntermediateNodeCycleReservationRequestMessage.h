#ifndef GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H

#include "base/RequestCycleMessage.h"

class IntermediateNodeCycleReservationRequestMessage :
    public RequestCycleMessage {

public:
    typedef shared_ptr<IntermediateNodeCycleReservationRequestMessage> Shared;
    typedef shared_ptr<const IntermediateNodeCycleReservationRequestMessage> ConstShared;

public:
    using RequestCycleMessage::RequestCycleMessage;

protected:
    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H
