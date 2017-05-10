#ifndef RESPONSEMESSAGE_H
#define RESPONSEMESSAGE_H


#include "../../base/transaction/TransactionMessage.h"


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
        const PathUUID &pathUUID,
        const OperationState state);

    ResponseMessage(
        BytesShared buffer);

    const OperationState state() const;

    const PathUUID pathUUID() const;

protected:
    typedef byte SerializedOperationState;
    const size_t kOffsetToInheritedBytes() const
        noexcept;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

private:
    OperationState mState;
    PathUUID mPathUUID;
};

#endif // RESPONSEMESSAGE_H
