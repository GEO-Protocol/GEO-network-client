#ifndef CoordinatorReservationResponseMessageMESSAGE_H
#define CoordinatorReservationResponseMessageMESSAGE_H


#include "base/ResponseMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"


class CoordinatorReservationResponseMessage:
    public ResponseMessage {

public:
    typedef shared_ptr<CoordinatorReservationResponseMessage> Shared;
    typedef shared_ptr<const CoordinatorReservationResponseMessage> ConstShared;

public:
    // TODO: Amount may be used as flag for approved/rejected
    // (true if amount > 0)
    CoordinatorReservationResponseMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const PathID &pathID,
        const OperationState state,
        const TrustLineAmount &reservedAmount=0);

    CoordinatorReservationResponseMessage(
        BytesShared buffer);

    const TrustLineAmount& amountReserved() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

protected:
    const MessageType typeID() const;

protected:
    TrustLineAmount mAmountReserved;
};

#endif // INTERMEDIATENODERESERVATIONRESPONSE_H
