#ifndef INTERMEDIATENODERESERVATIONRESPONSEMESSAGE_H
#define INTERMEDIATENODERESERVATIONRESPONSEMESSAGE_H


#include "base/ResponseMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"


class IntermediateNodeReservationResponseMessage:
    public ResponseMessage {

public:
    typedef shared_ptr<IntermediateNodeReservationResponseMessage> Shared;
    typedef shared_ptr<const IntermediateNodeReservationResponseMessage> ConstShared;

public:
    // TODO: Amount may be used as flag for approved/rejected
    // (true if amount > 0)
    IntermediateNodeReservationResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const PathUUID &pathUUID,
        const OperationState state,
        const TrustLineAmount &reservedAmount=0);

    IntermediateNodeReservationResponseMessage(
        BytesShared buffer);

    const TrustLineAmount& amountReserved() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

protected:
    const MessageType typeID() const;

    void deserializeFromBytes(
        BytesShared buffer);

protected:
    TrustLineAmount mAmountReserved;
};

#endif // INTERMEDIATENODERESERVATIONRESPONSEMESSAGE_H
