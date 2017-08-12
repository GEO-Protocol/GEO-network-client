#ifndef GEO_NETWORK_CLIENT_CYCLECLOSERINTERMEDIATENODETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLECLOSERINTERMEDIATENODETRANSACTION_H

#include "base/BasePaymentTransaction.h"
#include "../../../../cycles/CyclesManager.h"

class CycleCloserIntermediateNodeTransaction : public BasePaymentTransaction {

public:
    typedef shared_ptr<CycleCloserIntermediateNodeTransaction> Shared;
    typedef shared_ptr<const CycleCloserIntermediateNodeTransaction> ConstShared;

public:
    CycleCloserIntermediateNodeTransaction(
        const NodeUUID &currentNodeUUID,
        IntermediateNodeCycleReservationRequestMessage::ConstShared message,
        TrustLinesManager *trustLines,
        CyclesManager *cyclesManager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &log,
        TestingController *testingController);

    CycleCloserIntermediateNodeTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager* trustLines,
        CyclesManager *cyclesManager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &log,
        TestingController *testingController);

    TransactionResult::SharedConst run()
    noexcept;

    const NodeUUID& coordinatorUUID() const;

    const uint8_t cycleLength() const;

protected:
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStage();
    TransactionResult::SharedConst runCoordinatorRequestProcessingStage();
    TransactionResult::SharedConst runNextNeighborResponseProcessingStage();
    TransactionResult::SharedConst runReservationProlongationStage();
    TransactionResult::SharedConst runFinalPathConfigurationProcessingStage();
    // run after waiting on releasing amount by rollbacking conflicted transaction
    TransactionResult::SharedConst runCoordinatorRequestProcessingStageAgain();
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStageAgain();

protected:
    TransactionResult::SharedConst approve();

protected:
    void savePaymentOperationIntoHistory();

    bool checkReservationsDirections() const;

    const string logHeader() const;

protected:
    IntermediateNodeCycleReservationRequestMessage::ConstShared mMessage;

    TrustLineAmount mLastReservedAmount;
    NodeUUID mCoordinator;
    uint8_t mCycleLength;

    // fields, wor continue process coordinator request after releasing conflicted reservation
    // transaction on which reservation we pretend
    TransactionUUID mConflictedTransaction;
    NodeUUID mNextNode;
    NodeUUID mPreviousNode;
    TrustLineAmount mReservationAmount;

    // for resolving reservation conflicts
    CyclesManager *mCyclesManager;

private:
    const uint16_t kWaitingForReleasingAmountMSec = 50;
};


#endif //GEO_NETWORK_CLIENT_CYCLECLOSERINTERMEDIATENODETRANSACTION_H
