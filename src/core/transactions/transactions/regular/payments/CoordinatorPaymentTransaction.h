#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H

#include "../../../../common/Types.h"
#include "../../../../paths/lib/Path.h"

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../logger/Logger.h"

#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"
#include "../../../../network/messages/outgoing/payments/ReceiverInitPaymentMessage.h"
#include "../../../../network/messages/outgoing/payments/ReserveBalanceRequestMessage.h"

#include <map>


class CoordinatorPaymentTransaction:
    public BaseTransaction {

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
    const string logHeader() const;

protected:
    // Typedefs
    typedef boost::uuids::uuid PathUUID;

protected:
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
            const Path &path);

        void setNodeState(
            const uint8_t positionInPath,
            const NodeState state);

        const Path& path() const;
        const pair<NodeUUID, uint8_t> nextIntermediateNodeAndPos() const;
        const bool reservationRequestSentToAllNodes() const;
        const bool isWaitingForReservationResponse() const;
        const bool isReadyToSendNextReservationRequest() const;
        const bool isLastIntermediateNodeProcessed() const;

    protected:
        const Path mPath;
        vector<NodeState> mNodesStates;
        TrustLineAmount mMaxPathFlow;
    };

protected:
    // Stages handlers
    TransactionResult::SharedConst initTransaction();
    TransactionResult::SharedConst processReceiverResponse();
    TransactionResult::SharedConst tryBlockAmounts();

protected:
    // Results handlers
    TransactionResult::SharedConst resultOK();
    TransactionResult::SharedConst resultNoPaths();
    TransactionResult::SharedConst resultProtocolError();
    TransactionResult::SharedConst resultInsufficientFundsError();

protected:
    // Init operation helpers
    void addPathForFurtherProcessing(
        const Path& path);

    // Amounts reservation helpers
    void initAmountsReservationOnNextPath();

    PathStats* currentAmountReservationPathStats();

    TransactionResult::SharedConst sendNextAmountReservationRequest(
        PathStats* path);

    TransactionResult::SharedConst processRemoteNodeResponse();

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

protected:
    // shared
    TrustLinesManager *mTrustLines;
    Logger *mLog;
};
#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
