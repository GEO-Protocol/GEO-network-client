#ifndef GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H

#include "base/BasePaymentTransaction.h"
#include "base/PathStats.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../cycles/CyclesManager.h"

#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <thread>

class CycleCloserInitiatorTransaction : public BasePaymentTransaction {

public:
    typedef shared_ptr<CycleCloserInitiatorTransaction> Shared;
    typedef shared_ptr<const CycleCloserInitiatorTransaction> ConstShared;

public:
    CycleCloserInitiatorTransaction(
        const NodeUUID &kCurrentNodeUUID,
        Path::ConstShared path,
        TrustLinesManager *trustLines,
        CyclesManager *cyclesManager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &log,
        TestingController *testingController)
        noexcept;

    CycleCloserInitiatorTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustLines,
        CyclesManager *cyclesManager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &log,
        TestingController *testingController)
        throw (bad_alloc);

    TransactionResult::SharedConst run()
        noexcept;

    const NodeUUID& coordinatorUUID() const;

    const uint8_t cycleLength() const;

protected:
    // Stages handlers
    // TODO: Add throws specififcations
    TransactionResult::SharedConst runInitialisationStage();
    TransactionResult::SharedConst runAmountReservationStage ();
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStage();
    TransactionResult::SharedConst propagateVotesListAndWaitForVoutingResult();
    // run after waiting on releasing amount by rollbacking conflicted transaction
    TransactionResult::SharedConst runAmountReservationStageAgain();
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStageAgain();

protected:
    TransactionResult::SharedConst tryReserveNextIntermediateNodeAmount ();

    TransactionResult::SharedConst askNeighborToReserveAmount();

    TransactionResult::SharedConst processNeighborAmountReservationResponse();

    TransactionResult::SharedConst askNeighborToApproveFurtherNodeReservation();

    TransactionResult::SharedConst processNeighborFurtherReservationResponse();

    TransactionResult::SharedConst askRemoteNodeToApproveReservation(
        const NodeUUID &remoteNode,
        const byte remoteNodePosition,
        const NodeUUID &nextNodeAfterRemote);

    TransactionResult::SharedConst processRemoteNodeResponse();

protected:
    const string logHeader() const;

    void checkPath(
        const Path::ConstShared path);

    void sendFinalPathConfiguration(
        PathStats* pathStats,
        const TrustLineAmount &finalPathAmount);

    void savePaymentOperationIntoHistory();

protected:
    // Contains special stats data, such as current msx flow,
    // for path involved into the transaction.
    unique_ptr<PathStats> mPathStats;

    // fields, wor continue process coordinator request after releasing conflicted reservation
    // transaction on which reservation we pretend
    TransactionUUID mConflictedTransaction;
    NodeUUID mNextNode;
    NodeUUID mPreviousNode;
    TrustLineAmount mOutgoingAmount;
    TrustLineAmount mIncomingAmount;

    // for resolving reservation conflicts
    CyclesManager *mCyclesManager;

private:
    const uint16_t kWaitingForReleasingAmountMSec = 50;
};

#endif //GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H
