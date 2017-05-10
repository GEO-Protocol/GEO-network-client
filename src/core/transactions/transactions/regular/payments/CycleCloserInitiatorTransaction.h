#ifndef GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H

#include "base/BasePaymentTransaction.h"
#include "base/PathStats.h"

#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <unordered_set>

class CycleCloserInitiatorTransaction : public BasePaymentTransaction {

public:
    typedef shared_ptr<CycleCloserInitiatorTransaction> Shared;
    typedef shared_ptr<const CycleCloserInitiatorTransaction> ConstShared;

public:
    CycleCloserInitiatorTransaction(
        const NodeUUID &kCurrentNodeUUID,
        Path::ConstShared path,
        TrustLinesManager *trustLines,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *log)
        noexcept;

    CycleCloserInitiatorTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLines,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *log)
        throw (bad_alloc);

    TransactionResult::SharedConst run()
        noexcept;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

protected:
    // Stages handlers
    // TODO: Add throws specififcations
    TransactionResult::SharedConst runInitialisationStage();
    TransactionResult::SharedConst runAmountReservationStage ();
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStage();
    TransactionResult::SharedConst propagateVotesListAndWaitForVoutingResult();

protected:
    TransactionResult::SharedConst reject(
        const char *message = nullptr);

protected:
    TransactionResult::SharedConst tryReserveNextIntermediateNodeAmount (
        PathStats *pathStats);

    TransactionResult::SharedConst askNeighborToReserveAmount(
        const NodeUUID &neighbor,
        PathStats *pathStats);

    TransactionResult::SharedConst processNeighborAmountReservationResponse();

    TransactionResult::SharedConst askNeighborToApproveFurtherNodeReservation(
        const NodeUUID &neighbor,
        PathStats *pathStats);

    TransactionResult::SharedConst processNeighborFurtherReservationResponse();

    TransactionResult::SharedConst askRemoteNodeToApproveReservation(
        PathStats *pathStats,
        const NodeUUID &remoteNode,
        const byte remoteNodePosition,
        const NodeUUID &nextNodeAfterRemote);

    TransactionResult::SharedConst processRemoteNodeResponse();

protected:
    const string logHeader() const;

    void deserializeFromBytes(
        BytesShared buffer);

    void checkPath(
        const Path::ConstShared path);

protected:
    // Contains special stats data, such as current msx flow,
    // for path involved into the transaction.
    unique_ptr<PathStats> mPathStats;

    // minimum value of Coordinator outgoing amount to first intremediate node
    // and incoming amount from last intermediate node
    TrustLineAmount mInitialTransactionAmount;

    // Contains nodes that has been requrested final paths configuration.
    // for more details, see TODO
    unordered_set<NodeUUID> mNodesRequestedFinalConfiguration;
};

#endif //GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H
