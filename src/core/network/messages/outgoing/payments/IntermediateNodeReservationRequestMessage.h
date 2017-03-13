#ifndef GEO_NETWORK_CLIENT_INTERMEDIATENODERESERVATIONREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_INTERMEDIATENODERESERVATIONREQUESTMESSAGE_H

#include "base/RequestMessage.h"


class IntermediateNodeReservationRequestMessage:
    public RequestMessage {

public:
    typedef shared_ptr<IntermediateNodeReservationRequestMessage> Shared;
    typedef shared_ptr<const IntermediateNodeReservationRequestMessage> ConstShared;

public:
    using RequestMessage::RequestMessage;

protected:
    const MessageType typeID() const;
};

#endif // GEO_NETWORK_CLIENT_INTERMEDIATENODERESERVATIONREQUESTMESSAGE_H
