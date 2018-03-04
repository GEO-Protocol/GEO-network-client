#ifndef GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONRESPONSEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class FinalAmountsConfigurationResponseMessage : public TransactionMessage {

public:
    enum OperationState {
        Accepted = 1,
        Rejected = 2,
    };

public:
    typedef shared_ptr<FinalAmountsConfigurationResponseMessage> Shared;

public:
    FinalAmountsConfigurationResponseMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state);

    FinalAmountsConfigurationResponseMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const OperationState state() const;

protected:
    typedef byte SerializedOperationState;

    pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

private:
    OperationState mState;
};


#endif //GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONRESPONSEMESSAGE_H
