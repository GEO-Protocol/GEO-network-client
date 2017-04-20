#ifndef GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H

#include "../regular/payments/base/BasePaymentTransaction.h"

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
        Logger *log)
        noexcept;

    CycleCloserInitiatorTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLines,
        Logger *log)
        throw (bad_alloc);

    TransactionResult::SharedConst run()
        throw (RuntimeError, bad_alloc);

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

protected:
    // TODO: move it into separate *.h file.
    typedef boost::uuids::uuid PathUUID;

protected:

    // Describes payment path, it's nodes states,
    // and max flow through it.
    class PathStats {
    public:
        enum NodeState {
            ReservationRequestDoesntSent = 0,

            NeighbourReservationRequestSent,
            NeighbourReservationApproved,

            ReservationRequestSent,
            ReservationApproved,
            ReservationRejected,
        };

    public:
        PathStats (
            const Path::ConstShared path)
            noexcept;

        void setNodeState (
            const uint8_t positionInPath,
            const NodeState state)
            throw(ValueError);

        const TrustLineAmount &maxFlow () const
            noexcept;

        void shortageMaxFlow (
            const TrustLineAmount &kAmount)
            throw(ValueError);

        const Path::ConstShared path () const
            noexcept;

        const pair<NodeUUID, uint8_t> currentIntermediateNodeAndPos () const
            throw (NotFoundError);

        const pair<NodeUUID, uint8_t> nextIntermediateNodeAndPos () const
            throw (NotFoundError);

        const bool reservationRequestSentToAllNodes () const
            noexcept;

        const bool isNeighborAmountReserved () const
            noexcept;

        const bool isWaitingForNeighborReservationResponse () const
            noexcept;

        const bool isWaitingForNeighborReservationPropagationResponse () const
            noexcept;

        const bool isWaitingForReservationResponse () const
            noexcept;

        const bool isReadyToSendNextReservationRequest () const
            noexcept;

        const bool isLastIntermediateNodeProcessed () const
            noexcept;

        const bool isValid () const
            noexcept;

        void setUnusable ()
            noexcept;

    protected:
        const Path::ConstShared mPath;
        TrustLineAmount mMaxPathFlow;
        bool mIsValid;

        // Contains states of each node in the path.
        // See reservaions stage for the details.
        vector<NodeState> mIntermediateNodesStates;
    };

protected:
    // Stages handlers
    // TODO: Add throws specififcations
    TransactionResult::SharedConst runAmountReservationStage ();
    TransactionResult::SharedConst propagateVotesListAndWaitForConfigurationRequests ();
    TransactionResult::SharedConst runFinalParticipantsRequestsProcessingStage ();
    TransactionResult::SharedConst runVotesCheckingStage ();

protected:
    // Coordinator must return command result on transaction finishing.
    // Therefore this methods are overriden.
    TransactionResult::SharedConst approve();
    TransactionResult::SharedConst recover(
        const char *message = nullptr);
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

    TrustLineAmount totalReservedByAllPaths() const;

protected:
    const string logHeader() const;
    void deserializeFromBytes(
        BytesShared buffer);

protected:

    // Contains special stats data, such as current msx flow,
    // for all paths involved into the transaction.
    //unordered_map<PathUUID, unique_ptr<PathStats>, boost::hash<boost::uuids::uuid>> mPathsStats;

    unique_ptr<PathStats> mPathStats;

    // Contains nodes that has been requrested final paths configuration.
    // for more details, see TODO
    unordered_set<NodeUUID> mNodesRequestedFinalConfiguration;
};

#endif //GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H
