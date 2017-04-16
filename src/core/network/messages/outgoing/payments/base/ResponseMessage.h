#ifndef RESPONSEMESSAGE_H
#define RESPONSEMESSAGE_H


#include "../../../base/transaction/TransactionMessage.h"


class ResponseMessage:
    public TransactionMessage {

public:
    enum OperationState {
        Accepted = 1,
        Rejected = 2,
    };

public:
    ResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state);

    ResponseMessage(
        BytesShared buffer);

    const OperationState state() const;

protected:
    typedef byte SerializedOperationState;
    const size_t kOffsetToInheritedBytes();

protected:
    pair<BytesShared, size_t> serializeToBytes();
    void deserializeFromBytes(
        BytesShared buffer);

private:
    OperationState mState;
};

#endif // RESPONSEMESSAGE_H
