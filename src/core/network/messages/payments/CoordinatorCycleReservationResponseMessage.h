#ifndef GEO_NETWORK_CLIENT_COORDINATORCYCLERESERVATIONRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_COORDINATORCYCLERESERVATIONRESPONSEMESSAGE_H

#include "base/ResponseCycleMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

class CoordinatorCycleReservationResponseMessage :
    public ResponseCycleMessage {

public:
    typedef shared_ptr<CoordinatorCycleReservationResponseMessage> Shared;
    typedef shared_ptr<const CoordinatorCycleReservationResponseMessage> ConstShared;

public:
    // TODO: Amount may be used as flag for approved/rejected
    // (true if amount > 0)
    CoordinatorCycleReservationResponseMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state,
        const TrustLineAmount &reservedAmount=0);

    CoordinatorCycleReservationResponseMessage(
        BytesShared buffer);

    const TrustLineAmount& amountReserved() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

protected:
    const MessageType typeID() const;

protected:
    TrustLineAmount mAmountReserved;
};


#endif //GEO_NETWORK_CLIENT_COORDINATORCYCLERESERVATIONRESPONSEMESSAGE_H
