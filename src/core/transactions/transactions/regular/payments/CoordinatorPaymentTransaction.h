#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H


#include "base/BasePaymentTransaction.h"

#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"
#include "../../../../network/messages/outgoing/payments/ReceiverInitPaymentMessage.h"
#include "../../../../network/messages/outgoing/payments/ReserveBalanceRequestMessage.h"

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
    pair<BytesShared, size_t> serializeToBytes();

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
            ReservationRequestSent,
            ReservationApproved,
            ReservationDisaproved,
        };

    public:
        PathStats(
            Path::ConstShared path);

        void setNodeState(
            const uint8_t positionInPath,
            const NodeState state);

        const Path::ConstShared path() const;
        const pair<NodeUUID, uint8_t> nextIntermediateNodeAndPos() const;
        const bool reservationRequestSentToAllNodes() const;
        const bool isWaitingForReservationResponse() const;
        const bool isReadyToSendNextReservationRequest() const;
        const bool isLastIntermediateNodeProcessed() const;

    protected:
        Path::ConstShared mPath;
        vector<NodeState> mIntermediateNodesStates;
        TrustLineAmount mMaxPathFlow;
    };

protected:
    // Stages handlers
    TransactionResult::SharedConst initTransaction();
    TransactionResult::SharedConst processReceiverResponse();
    TransactionResult::SharedConst tryReserveAmounts();

protected:
    // Results handlers
    TransactionResult::SharedConst resultOK();
    TransactionResult::SharedConst resultNoPathsError();
    TransactionResult::SharedConst resultProtocolError();
    TransactionResult::SharedConst resultNoResponseError();
    TransactionResult::SharedConst resultInsufficientFundsError();

protected:
    const string logHeader() const;

    // Init operation helpers
    void addPathForFurtherProcessing(
        Path::ConstShared path);

    // Amounts reservation helpers
    TransactionResult::SharedConst tryReserveNextNodeAmount(
        PathStats* path);

    TransactionResult::SharedConst sendReservationRequest(
        PathStats *pathStats,
        const NodeUUID &remoteNode,
        const byte remoteNodePosition,
        const NodeUUID &nextNodeAfterRemote,
        const TrustLineAmount &amount);

    TransactionResult::SharedConst processRemoteNodeResponse();
    TransactionResult::SharedConst tryProcessNextPath();

    void initAmountsReservationOnNextPath();

    PathStats* currentAmountReservationPathStats();
    void switchToNextPath();

    // Other
    void deserializeFromBytes(
        BytesShared buffer);

protected:
    // local
    CreditUsageCommand::Shared mCommand;
    map<PathUUID, unique_ptr<PathStats>> mPathsStats;

    // Used in amount reservations stage.
    // Contains identifier of the path,
    // that was processed last, and potenially,
    // is waiting for request appriving.
    PathUUID mCurrentAmountReservingPathIdentifier;
    byte mCurrentAmountReservingPathIdentifierIndex;

    byte mReservationsStage;
    TrustLineAmount mAlreadyReservedAmountOnAllPaths;
};
#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
