#ifndef GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTRESPONSEMESSAGE_H

#include "base/ResponseMessage.h"


class ReceiverInitPaymentResponseMessage:
    public ResponseMessage {

public:
    typedef shared_ptr<ReceiverInitPaymentResponseMessage> Shared;
    typedef shared_ptr<const ReceiverInitPaymentResponseMessage> ConstShared;

public:
    using  ResponseMessage::ResponseMessage;

protected:
    const MessageType typeID() const;
};

#endif //GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTRESPONSEMESSAGE_H
