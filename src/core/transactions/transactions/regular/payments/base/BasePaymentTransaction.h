#ifndef BASEPAYMENTTRANSACTION_H
#define BASEPAYMENTTRANSACTION_H


#include "../../../base/BaseTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../paths/lib/Path.h"
#include "../../../../../logger/Logger.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../io/storage/StorageHandler.h"
#include "../../../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"

#include "../../../../../network/messages/payments/ReceiverInitPaymentRequestMessage.h"
#include "../../../../../network/messages/payments/ReceiverInitPaymentResponseMessage.h"
#include "../../../../../network/messages/payments/CoordinatorReservationRequestMessage.h"
#include "../../../../../network/messages/payments/CoordinatorReservationResponseMessage.h"
#include "../../../../../network/messages/payments/IntermediateNodeReservationRequestMessage.h"
#include "../../../../../network/messages/payments/IntermediateNodeReservationResponseMessage.h"
#include "../../../../../network/messages/payments/CoordinatorCycleReservationRequestMessage.h"
#include "../../../../../network/messages/payments/CoordinatorCycleReservationResponseMessage.h"
#include "../../../../../network/messages/payments/IntermediateNodeCycleReservationRequestMessage.h"
#include "../../../../../network/messages/payments/IntermediateNodeCycleReservationResponseMessage.h"
#include "../../../../../network/messages/payments/ParticipantsVotesMessage.h"
#include "../../../../../network/messages/payments/VotesStatusRequestMessage.hpp"
#include "../../../../../network/messages/payments/FinalPathConfigurationMessage.h"
#include "../../../../../network/messages/payments/FinalPathCycleConfigurationMessage.h"
#include "../../../../../network/messages/payments/TTLPolongationMessage.h"

#include "PathStats.h"

namespace signals = boost::signals2;

// TODO: Add restoring of the reservations after transaction deserialization.
class BasePaymentTransaction:
    public BaseTransaction {

public:
    typedef signals::signal<void(vector<NodeUUID> &contractorUUID)> BuildCycleThreeNodesSignal;
    typedef signals::signal<void(vector<pair<NodeUUID, NodeUUID>> &debtorsAndCreditors)> BuildCycleFourNodesSignal;
    typedef signals::signal<void()> CycleWasClosedSignal;

public:
    BasePaymentTransaction(
        const TransactionType type,
        const NodeUUID &currentNodeUUID,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *log);

    BasePaymentTransaction(
        const TransactionType type,
        const TransactionUUID &transactionUUID,
        const NodeUUID &currentNodeUUID,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *log);

    BasePaymentTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *log);

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    enum Stages {
        Coordinator_Initialisation = 1,
        Coordinator_ReceiverResourceProcessing,
        Coordinator_ReceiverResponseProcessing,
        Coordinator_AmountReservation,
        Coordinator_ShortPathAmountReservationResponseProcessing,
        Coordinator_PreviousNeighborRequestProcessing,

        Receiver_CoordinatorRequestApproving,
        Receiver_AmountReservationsProcessing,

        IntermediateNode_PreviousNeighborRequestProcessing,
        IntermediateNode_CoordinatorRequestProcessing,
        IntermediateNode_NextNeighborResponseProcessing,
        IntermediateNode_ReservationProlongation,

        Common_VotesChecking,
        Common_FinalPathConfigurationChecking,
        Common_Recovery,
        Common_ClarificationTransaction

    };

    enum VotesRecoveryStages {
        Common_PrepareNodesListToCheckVotes,
        Common_CheckCoordinatorVotesStage,
        Common_CheckIntermediateNodeVotesStage
    };

protected:
    // TODO: move it into separate *.h file.
    typedef uint16_t PathUUID;

    // Stages handlers
    virtual TransactionResult::SharedConst runVotesCheckingStage();
    virtual TransactionResult::SharedConst runVotesConsistencyCheckingStage();
    virtual TransactionResult::SharedConst runFinalPathConfigurationProcessingStage();
    virtual TransactionResult::SharedConst runTTLTransactionResponce();

    virtual TransactionResult::SharedConst approve();
    virtual TransactionResult::SharedConst recover(
        const char *message = nullptr);
    virtual TransactionResult::SharedConst reject(
        const char *message = nullptr);
    virtual TransactionResult::SharedConst cancel(
        const char *message = nullptr);

    TransactionResult::SharedConst exitWithResult(
        const TransactionResult::SharedConst result,
        const char *message=nullptr);

    TransactionResult::SharedConst runVotesRecoveryParentStage();
    TransactionResult::SharedConst sendVotesRequestMessageAndWaitForResponse(const NodeUUID &contractorUUID);
    TransactionResult::SharedConst runPrepareListNodesToCheckNodes();
    TransactionResult::SharedConst runCheckCoordinatorVotesStage();
    TransactionResult::SharedConst runCheckIntermediateNodeVotesSage();


protected:
    const bool reserveOutgoingAmount(
        const NodeUUID &neighborNode,
        const TrustLineAmount& amount,
        const PathUUID &pathUUID);

    const bool reserveIncomingAmount(
        const NodeUUID &neighborNode,
        const TrustLineAmount& amount,
        const PathUUID &pathUUID);

    const bool shortageReservation(
        const NodeUUID kContractor,
        const AmountReservation::ConstShared kReservation,
        const TrustLineAmount &kNewAmount,
        const PathUUID &pathUUID);

    void saveVotes();

    void commit();

    void rollBack();

    void rollBack(
        const PathUUID &pathUUID);

    uint32_t maxNetworkDelay (
        const uint16_t totalHopsCount) const;

    uint32_t maxCoordinatorResponseTimeout() const;

    const bool contextIsValid(
        Message::MessageType messageType,
        bool showErrorMessage = true) const;

    const bool positiveVoteIsPresent (
        const ParticipantsVotesMessage::ConstShared kMessage) const;

    void propagateVotesMessageToAllParticipants (
        const ParticipantsVotesMessage::Shared kMessage) const;

    void dropReservationsOnPath(
        PathStats *pathStats,
        PathUUID pathUUID);

    void sendFinalPathConfiguration(
        PathStats* pathStats,
        PathUUID pathUUID,
        const TrustLineAmount &finalPathAmount);

    size_t reservationsSizeInBytes() const;

protected:
    // Specifies how long node must wait for the response from the remote node.
    // This timeout must take into account also that remote node may process other transaction,
    // and may be too busy to response.
    // (it is not only network transfer timeout).
    static const uint16_t kMaxMessageTransferLagMSec = 1500; // milliseconds

    // Specifies how long node must wait for the resources from other transaction
    static const uint16_t kMaxResourceTransferLagMSec = 2000; //

    static const auto kMaxPathLength = 7;

public:
    mutable BuildCycleThreeNodesSignal mBuildCycleThreeNodesSignal;

    mutable BuildCycleFourNodesSignal mBuildCycleFourNodesSignal;

    mutable CycleWasClosedSignal cycleWasClosedSignal;

protected:
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;

    // If true - votes check stage has been processed and transaction has been approved.
    // In this case transaction can't be simply rolled back.
    // It may only be canceled through recover stage.
    //
    // If false - transaction wasn't approved yet.
    bool mTransactionIsVoted;

    // Votes message cant be signed right after it is received.
    // Additional message must be received from the coordinator,
    // so the votes message must be saved for further processing.
    ParticipantsVotesMessage::Shared mParticipantsVotesMessage;

    map<NodeUUID, vector<pair<PathUUID, AmountReservation::ConstShared>>> mReservations;

    // Votes recovery
    vector<NodeUUID> mNodesToCheckVotes;
    NodeUUID mCurrentNodeToCheckVotes;

};

#endif // BASEPAYMENTTRANSACTION_H
