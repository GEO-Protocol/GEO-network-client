#ifndef GEO_NETWORK_CLIENT_RESERVEBALANCEREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_RESERVEBALANCEREQUESTMESSAGE_H

#include "../../outgoing/payments/ReceiverInitPaymentMessage.h"


class ReserveBalanceRequestMessage:
    public ReceiverInitPaymentMessage {

public:
    typedef shared_ptr<ReserveBalanceRequestMessage> Shared;
    typedef shared_ptr<const ReserveBalanceRequestMessage> ConstShared;

public:
    using ReceiverInitPaymentMessage::ReceiverInitPaymentMessage;

private:
    const MessageType typeID() const;
};

#endif // GEO_NETWORK_CLIENT_RESERVEBALANCEREQUESTMESSAGE_H
