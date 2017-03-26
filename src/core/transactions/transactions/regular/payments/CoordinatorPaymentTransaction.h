#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H


#include "base/BasePaymentTransaction.h"

#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"

#include "../../../../network/messages/outgoing/payments/ReceiverInitPaymentRequestMessage.h"
#include "../../../../network/messages/outgoing/payments/ReceiverInitPaymentResponseMessage.h"
#include "../../../../network/messages/outgoing/payments/CoordinatorReservationRequestMessage.h"
#include "../../../../network/messages/outgoing/payments/CoordinatorReservationResponseMessage.h"
#include "../../../../network/messages/outgoing/payments/IntermediateNodeReservationRequestMessage.h"
#include "../../../../network/messages/outgoing/payments/IntermediateNodeReservationResponseMessage.h"
#include "../../../../network/messages/outgoing/payments/ParticipantsVotesMessage.h"

#include <map>


class CoordinatorPaymentTransaction:
    public BasePaymentTransaction {

public:
    typedef shared_ptr<CoordinatorPaymentTransaction> Shared;
    typedef shared_ptr<const CoordinatorPaymentTransaction> ConstShared;

public:
    CoordinatorPaymentTransaction(
        const NodeUUID &currentNodeUUID,
        CreditUsageCommand::Shared command,
        TrustLinesManager *trustLines,
        Logger *log);

    CoordinatorPaymentTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLines,
        Logger *log);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes() const;

protected:
    typedef boost::uuids::uuid PathUUID;

protected:
    /**
     * Describes payment path, it's nodes states, and max flow through it.
     */
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
        PathStats(
            Path::ConstShared path);

        void setNodeState(
            const uint8_t positionInPath,
            const NodeState state);

        const TrustLineAmount& maxFlow() const;
        void setMaxFlow(
             const TrustLineAmount &amount);

        const Path::ConstShared path() const;
        const pair<NodeUUID, uint8_t> currentIntermediateNodeAndPos() const;
        const pair<NodeUUID, uint8_t> nextIntermediateNodeAndPos() const;
        const bool reservationRequestSentToAllNodes() const;

        const bool isNeighborAmountReserved() const;
        const bool isWaitingForNeighborReservationResponse() const;
        const bool isWaitingForNeighborReservationPropagationResponse() const;
        const bool isWaitingForReservationResponse() const;

        const bool isReadyToSendNextReservationRequest() const;
        const bool isLastIntermediateNodeProcessed() const;

        const bool isValid() const;
        void setUnusable();

    protected:
        Path::ConstShared mPath;
        vector<NodeState> mIntermediateNodesStates;
        TrustLineAmount mMaxPathFlow;
        bool mIsValid;
    };

protected:
    enum Stages {
        Initialisation = 1,
        ReceiverResponseProcessing,
        AmountReservation,
        VotesListChecking,
    };


    // Stages handlers
    TransactionResult::SharedConst runPaymentInitialisationStage ();
    TransactionResult::SharedConst runReceiverResponseProcessingStage ();
    TransactionResult::SharedConst runAmountReservationStage ();
    TransactionResult::SharedConst propagateVotesList();

    // Coordinator node must return command result on transaction finishing.
    // Therefore this methods are overriden.
    TransactionResult::SharedConst approve();
    TransactionResult::SharedConst recover();
    TransactionResult::SharedConst reject(
        const char *message = nullptr);

protected:
    // Results handlers
    TransactionResult::SharedConst resultOK();
    TransactionResult::SharedConst resultNoPathsError();
    TransactionResult::SharedConst resultProtocolError();
    TransactionResult::SharedConst resultNoResponseError();
    TransactionResult::SharedConst resultInsufficientFundsError();
    TransactionResult::SharedConst resultNoConsensusError();

protected:
    // Init operation helpers
    void addPathForFurtherProcessing(
        Path::ConstShared path);

    // Amounts reservation helpers
    void initAmountsReservationOnNextPath();
    void switchToNextPath();

    PathStats* currentAmountReservationPathStats();
    TransactionResult::SharedConst tryProcessNextPath();

    TransactionResult::SharedConst tryReserveNextNodeAmount(
        PathStats* pathStats);

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

    TrustLineAmount totalReservedByAllPaths() const;

protected:
    // local
    CreditUsageCommand::Shared mCommand;
    map<PathUUID, unique_ptr<PathStats>> mPathsStats;

    // Used in amount reservations stage.
    // Contains identifier of the path,
    // that was processed last, and potenially,
    // is waiting for request appriving.
    PathUUID mCurrentAmountReservingPathIdentifier;

    byte mReservationsStage;
};
#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
