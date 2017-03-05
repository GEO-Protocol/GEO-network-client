#ifndef INTERMEDIATENODERESERVATIONRESPONSE_H
#define INTERMEDIATENODERESERVATIONRESPONSE_H


#include "../../base/transaction/TransactionMessage.h"


class IntermediateNodeReservationResponse:
    public TransactionMessage {

public:
    enum OperationState {
        Accepted = 1,
        Rejected = 2,
    };

    typedef byte SerializedOperationState;
    typedef shared_ptr<IntermediateNodeReservationResponse> Shared;
    typedef shared_ptr<const IntermediateNodeReservationResponse> ConstShared;

public:
    IntermediateNodeReservationResponse(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state);

    IntermediateNodeReservationResponse(
        BytesShared buffer);

    const OperationState state() const;

private:
    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    mutable OperationState mState;
};

#endif // INTERMEDIATENODERESERVATIONRESPONSE_H
