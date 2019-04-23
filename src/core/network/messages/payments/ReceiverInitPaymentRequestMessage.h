#ifndef GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTREQUESTMESSAGE_H

#include "base/RequestMessage.h"

class ReceiverInitPaymentRequestMessage:
    public RequestMessage {

public:
    typedef shared_ptr<ReceiverInitPaymentRequestMessage> Shared;
    typedef shared_ptr<const ReceiverInitPaymentRequestMessage> ConstShared;

public:
    ReceiverInitPaymentRequestMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> &senderAddresses,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount,
        const string payload);

    ReceiverInitPaymentRequestMessage(
        BytesShared buffer);

    const MessageType typeID() const override;

    const string payload() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const override;

private:
    string mPayload;
};
#endif //GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTMESSAGE_H
