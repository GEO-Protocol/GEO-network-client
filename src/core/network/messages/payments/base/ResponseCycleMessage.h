#ifndef GEO_NETWORK_CLIENT_RESPONSECYCLEMESSAGE_H
#define GEO_NETWORK_CLIENT_RESPONSECYCLEMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

class ResponseCycleMessage : public TransactionMessage {

public:
    enum OperationState {
        Accepted = 1,
        Rejected = 2,
        RejectedBecauseReservations = 3,
        NextNodeInaccessible = 4
    };

public:
    ResponseCycleMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state);

    ResponseCycleMessage(
        BytesShared buffer);

    const OperationState state() const;

protected:
    typedef byte SerializedOperationState;
    const size_t kOffsetToInheritedBytes() const
    noexcept;

    pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

private:
    OperationState mState;
};


#endif //GEO_NETWORK_CLIENT_RESPONSECYCLEMESSAGE_H
