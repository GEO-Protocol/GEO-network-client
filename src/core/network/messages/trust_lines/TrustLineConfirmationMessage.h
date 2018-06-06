#ifndef GEO_NETWORK_CLIENT_TRUSTLINECONFIRMATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINECONFIRMATIONMESSAGE_H

#include "../base/transaction/ConfirmationMessage.h"

class TrustLineConfirmationMessage : public ConfirmationMessage {

public:
    typedef shared_ptr<TrustLineConfirmationMessage> Shared;

public:
    TrustLineConfirmationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        bool gateway,
        const OperationState state);

    TrustLineConfirmationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const bool gateway() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

private:
    bool mGateway;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINECONFIRMATIONMESSAGE_H
