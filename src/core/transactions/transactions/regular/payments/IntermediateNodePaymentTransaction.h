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
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &log,
        SubsystemsController *subsystemsController);

    IntermediateNodePaymentTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager* trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &log,
        SubsystemsController *subsystemsController);

    TransactionResult::SharedConst run()
        noexcept;

protected:
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStage();
    TransactionResult::SharedConst runCoordinatorRequestProcessingStage();
    TransactionResult::SharedConst runNextNeighborResponseProcessingStage();
    TransactionResult::SharedConst runFinalPathConfigurationProcessingStage();
    TransactionResult::SharedConst runReservationProlongationStage();
    TransactionResult::SharedConst runClarificationOfTransactionBeforeVoting();
    TransactionResult::SharedConst runFinalAmountsConfigurationConfirmation();
    TransactionResult::SharedConst runClarificationOfTransactionDuringVoting();
    TransactionResult::SharedConst runVotesCheckingStageWithCoordinatorClarification();

protected:
    // Intermediate node must launch closing cycles 3 and 4 transactions.
    // Therefore this methods are overridden.
    TransactionResult::SharedConst approve();

protected:
    void shortageReservationsOnPath(
        const PathID pathID,
        const TrustLineAmount &amount);

    void savePaymentOperationIntoHistory(
        IOTransaction::Shared ioTransaction);

    bool checkReservationsDirections() const;

    const string logHeader() const;

protected:
    IntermediateNodeReservationRequestMessage::ConstShared mMessage;

    TrustLineAmount mLastReservedAmount;
    NodeUUID mCoordinator;
    PathID mLastProcessedPath;
};


#endif // INTERMEDIATENODEPAYMENTTRANSACTION_H
