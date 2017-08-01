#ifndef GEO_NETWORK_CLIENT_INTERMEDIATENODERESERVATIONREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_INTERMEDIATENODERESERVATIONREQUESTMESSAGE_H

#include "FinalAmountsConfigurationMessage.h"


class IntermediateNodeReservationRequestMessage:
    public FinalAmountsConfigurationMessage {

public:
    typedef shared_ptr<IntermediateNodeReservationRequestMessage> Shared;
    typedef shared_ptr<const IntermediateNodeReservationRequestMessage> ConstShared;

public:
    using FinalAmountsConfigurationMessage::FinalAmountsConfigurationMessage;

protected:
    const MessageType typeID() const;
};

#endif // GEO_NETWORK_CLIENT_INTERMEDIATENODERESERVATIONREQUESTMESSAGE_H
