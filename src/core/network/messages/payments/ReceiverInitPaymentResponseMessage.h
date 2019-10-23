#ifndef GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTRESPONSEMESSAGE_H

#include "base/ResponseMessage.h"


class ReceiverInitPaymentResponseMessage:
    public ResponseMessage {

public:
    typedef shared_ptr<ReceiverInitPaymentResponseMessage> Shared;
    typedef shared_ptr<const ReceiverInitPaymentResponseMessage> ConstShared;

public:
    ReceiverInitPaymentResponseMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> &senderAddresses,
        const TransactionUUID &transactionUUID,
        const OperationState state);

    ReceiverInitPaymentResponseMessage(
        BytesShared buffer);

    const MessageType typeID() const override;
};

#endif //GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTRESPONSEMESSAGE_H
