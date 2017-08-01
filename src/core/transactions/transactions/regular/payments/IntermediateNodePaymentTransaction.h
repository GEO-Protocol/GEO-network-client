#ifndef INTERMEDIATENODEPAYMENTTRANSACTION_H
#define INTERMEDIATENODEPAYMENTTRANSACTION_H

#include "base/BasePaymentTransaction.h"
#include <unordered_set>

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
        Logger &log);

    IntermediateNodePaymentTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager* trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &log);

    TransactionResult::SharedConst run()
        noexcept;

protected:
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStage();
    TransactionResult::SharedConst runCoordinatorRequestProcessingStage();
    TransactionResult::SharedConst runNextNeighborResponseProcessingStage();
    TransactionResult::SharedConst runFinalPathConfigurationProcessingStage();
    TransactionResult::SharedConst runReservationProlongationStage();
    TransactionResult::SharedConst runClarificationOfTransaction();
    TransactionResult::SharedConst runFinalAmountsConfigurationConfirmation();
    TransactionResult::SharedConst runVotesCheckingStageWithCoordinatorClarification();

protected:
    // Intermediate node must launch close cyles 3 and 4 transactions.
    // Therefore this methods are overriden.
    TransactionResult::SharedConst approve();

protected:
    // Updates all reservations according to finalAmounts
    // if some reservations will be found, pathUUIDs of which are absent in finalAmounts, returns false,
    // otherwise returns true
    bool updateReservations(
        const vector<pair<PathUUID, ConstSharedTrustLineAmount>> &finalAmounts);

    // Returns reservation pathID, which was updated, if reservation was dropped, returns 0
    PathUUID updateReservation(
        const NodeUUID &contractorUUID,
        pair<PathUUID, AmountReservation::ConstShared> &reservation,
        const vector<pair<PathUUID, ConstSharedTrustLineAmount>> &finalAmounts);

    void runBuildFourNodesCyclesSignal();

    void runBuildThreeNodesCyclesSignal();

    void savePaymentOperationIntoHistory();

    const string logHeader() const;

protected:
    IntermediateNodeReservationRequestMessage::ConstShared mMessage;

    TrustLineAmount mLastReservedAmount;
    NodeUUID mCoordinator;
    PathUUID mLastProcessedPath;

    // used for history saving of total amount during transaction
    TrustLineAmount mTotalReservedAmount;
};


#endif // INTERMEDIATENODEPAYMENTTRANSACTION_H
