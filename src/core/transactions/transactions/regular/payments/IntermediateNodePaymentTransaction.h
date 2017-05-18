#ifndef INTERMEDIATENODEPAYMENTTRANSACTION_H
#define INTERMEDIATENODEPAYMENTTRANSACTION_H


#include "base/BasePaymentTransaction.h"
#include "../../cycles/FourNodes/CyclesFourNodesInitTransaction.h"
#include "../../cycles/ThreeNodes/CyclesThreeNodesInitTransaction.h"


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
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *log);

    IntermediateNodePaymentTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager* trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger* log);

    TransactionResult::SharedConst run()
        noexcept;

    pair<BytesShared, size_t> serializeToBytes();

protected:
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStage();
    TransactionResult::SharedConst runCoordinatorRequestProcessingStage();
    TransactionResult::SharedConst runNextNeighborResponseProcessingStage();
    TransactionResult::SharedConst runReservationProlongationStage();
    TransactionResult::SharedConst runClarificationOfTransaction();
    TransactionResult::SharedConst runVotesCheckingStageWithCoordinatorClarification();

protected:
    // Intermediate node must launch close cyles 3 and 4 transactions.
    // Therefore this methods are overriden.
    TransactionResult::SharedConst approve();

protected:
    void launchFourCyclesClosingTransactions();

    void launchThreeCyclesClosingTransactions();

    void deserializeFromBytes(
        BytesShared buffer);

    const string logHeader() const;

protected:
    IntermediateNodeReservationRequestMessage::ConstShared mMessage;

    TrustLineAmount mLastReservedAmount;
    NodeUUID mCoordinator;
    PathUUID mLastProcessedPath;
};


#endif // INTERMEDIATENODEPAYMENTTRANSACTION_H
