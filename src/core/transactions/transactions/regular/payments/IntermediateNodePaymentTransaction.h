#ifndef INTERMEDIATENODEPAYMENTTRANSACTION_H
#define INTERMEDIATENODEPAYMENTTRANSACTION_H


#include "base/BasePaymentTransaction.h"


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
        PaymentOperationStateHandler *paymentOperationStateHandler,
        Logger *log);

    IntermediateNodePaymentTransaction(
        BytesShared buffer,
        TrustLinesManager* trustLines,
        PaymentOperationStateHandler *paymentOperationStateHandler,
        Logger* log);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes();

protected:
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStage();
    TransactionResult::SharedConst runCoordinatorRequestProcessingStage();
    TransactionResult::SharedConst runNextNeighborResponseProcessingStage();
    TransactionResult::SharedConst runReservationProlongationStage();
    TransactionResult::SharedConst runPreviousNeighborVoutesRequestStage();
    TransactionResult::SharedConst runCheckPreviousNeighborVoutesStage();
    TransactionResult::SharedConst sendVoutesRequestMessageAndWaitForResponse(const NodeUUID &contractorUUID);



protected:
    void deserializeFromBytes(
        BytesShared buffer);

    const string logHeader() const;

protected:
    const IntermediateNodeReservationRequestMessage::ConstShared mMessage;

    TrustLineAmount mLastReservedAmount;
    NodeUUID mCoordinator;
};


#endif // INTERMEDIATENODEPAYMENTTRANSACTION_H
