#ifndef INTERMEDIATENODEPAYMENTTRANSACTION_H
#define INTERMEDIATENODEPAYMENTTRANSACTION_H


#include "base/BasePaymentTransaction.h"

#include "../../../../network/messages/outgoing/payments/CoordinatorReservationRequestMessage.h"
#include "../../../../network/messages/outgoing/payments/CoordinatorReservationResponseMessage.h"
#include "../../../../network/messages/outgoing/payments/IntermediateNodeReservationRequestMessage.h"
#include "../../../../network/messages/outgoing/payments/IntermediateNodeReservationResponseMessage.h"


class IntermediateNodePaymentTransaction:
    public BasePaymentTransaction {

public:
    typedef shared_ptr<IntermediateNodePaymentTransaction> Shared;
    typedef shared_ptr<const IntermediateNodePaymentTransaction> ConstShared;

public:
    IntermediateNodePaymentTransaction(
        const NodeUUID &currentNodeUUID,
        IntermediateNodeReservationRequestMessage::ConstShared message,
        TrustLinesManager *trustLines,
        Logger *log);

    IntermediateNodePaymentTransaction(
        BytesShared buffer,
        TrustLinesManager* trustLines,
        Logger* log);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes();

protected:
    // Used as timeout for asking coordinator about what to do with reservation:
    // prolong or drop and exit.
    static const uint16_t kCoordinatorPingTimeoutMSec = 3000;

protected:
    enum Stages {
        PreviousNeighborRequestProcessing = 1,
        CoordinatorRequestProcessing,
        NextNeighborResponseProcessing,
        ReservationProlongation,
    };

    TransactionResult::SharedConst processPreviousNeighborRequest();
    TransactionResult::SharedConst processCoordinatorRequest();
    TransactionResult::SharedConst processNextNeighborResponse();
    TransactionResult::SharedConst processReservationProlongation();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    const string logHeader() const;

protected:
    const IntermediateNodeReservationRequestMessage::ConstShared mMessage;
    NodeUUID mCoordinator;
    TrustLineAmount mLastReservedAmount;
};


#endif // INTERMEDIATENODEPAYMENTTRANSACTION_H
