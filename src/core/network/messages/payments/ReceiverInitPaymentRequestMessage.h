#ifndef GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTREQUESTMESSAGE_H

#include "base/RequestMessage.h"


class ReceiverInitPaymentRequestMessage:
    public RequestMessage {

public:
    typedef shared_ptr<ReceiverInitPaymentRequestMessage> Shared;
    typedef shared_ptr<const ReceiverInitPaymentRequestMessage> ConstShared;

public:
    using RequestMessage::RequestMessage;

private:
    const MessageType typeID() const;
};
#endif //GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTMESSAGE_H
