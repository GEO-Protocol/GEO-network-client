#ifndef RESPONSEMESSAGE_H
#define RESPONSEMESSAGE_H


#include "../../base/transaction/TransactionMessage.h"


class ResponseMessage:
    public TransactionMessage {

public:
    enum OperationState {
        Accepted = 1,
        Rejected = 2,
        // used for immediately closing transaction
        Closed = 3,
        NextNodeInaccessible = 4
    };

public:
    ResponseMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const PathID &pathID,
        const OperationState state);

    ResponseMessage(
        BytesShared buffer);

    const OperationState state() const;

    const PathID pathID() const;

protected:
    typedef byte SerializedOperationState;
    const size_t kOffsetToInheritedBytes() const
        noexcept;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

private:
    OperationState mState;
    PathID mPathID;
};

#endif // RESPONSEMESSAGE_H
