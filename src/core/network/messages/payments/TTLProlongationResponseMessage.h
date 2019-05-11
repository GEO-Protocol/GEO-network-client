#ifndef GEO_NETWORK_CLIENT_TTLPROLONGATIONRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_TTLPROLONGATIONRESPONSEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class TTLProlongationResponseMessage : public TransactionMessage {

public:
    typedef shared_ptr<TTLProlongationResponseMessage> Shared;
    typedef shared_ptr<const TTLProlongationResponseMessage> ConstShared;

public:
    enum OperationState {
        Continue = 1,
        Finish = 2,
    };

public:
    TTLProlongationResponseMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> &senderAddresses,
        const TransactionUUID &transactionUUID,
        const OperationState state);

    TTLProlongationResponseMessage(
        BytesShared buffer);

    const Message::MessageType typeID() const override;

    const OperationState state() const;

protected:
    typedef byte SerializedOperationState;

    pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    OperationState mState;
};


#endif //GEO_NETWORK_CLIENT_TTLPROLONGATIONRESPONSEMESSAGE_H
