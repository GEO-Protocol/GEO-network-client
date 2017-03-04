#ifndef INTERMEDIATENODEPAYMENTTRANSACTION_H
#define INTERMEDIATENODEPAYMENTTRANSACTION_H


#include "base/BasePaymentTransaction.h"

#include "../../../../network/messages/outgoing/payments/ReserveBalanceRequestMessage.h"


class IntermediateNodePaymentTransaction:
    public BasePaymentTransaction {

public:
    typedef shared_ptr<IntermediateNodePaymentTransaction> Shared;
    typedef shared_ptr<const IntermediateNodePaymentTransaction> ConstShared;

public:
    IntermediateNodePaymentTransaction(
        const NodeUUID &currentNodeUUID,
        ReserveBalanceRequestMessage::ConstShared message,
        TrustLinesManager *trustLines,
        Logger *log);

    IntermediateNodePaymentTransaction(
        BytesShared buffer,
        TrustLinesManager* trustLines,
        Logger* log);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes();

protected:
    // Stages handlers
    TransactionResult::SharedConst initOperation();

protected:
    TransactionResult::SharedConst processReservationRequestFromNeighbour();
    TransactionResult::SharedConst processReservationRequestFromCoordinator();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    const string logHeader() const;

protected:
    const ReserveBalanceRequestMessage::ConstShared mMessage;
};


#endif // INTERMEDIATENODEPAYMENTTRANSACTION_H
