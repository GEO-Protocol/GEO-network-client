#ifndef BASEPAYMENTTRANSACTION_H
#define BASEPAYMENTTRANSACTION_H


#include "../../../base/BaseTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../paths/lib/Path.h"
#include "../../../../../logger/Logger.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../io/storage/StorageHandler.h"

#include "../../../../../network/messages/payments/ReceiverInitPaymentRequestMessage.h"
#include "../../../../../network/messages/payments/ReceiverInitPaymentResponseMessage.h"
#include "../../../../../network/messages/payments/CoordinatorReservationRequestMessage.h"
#include "../../../../../network/messages/payments/CoordinatorReservationResponseMessage.h"
#include "../../../../../network/messages/payments/IntermediateNodeReservationRequestMessage.h"
#include "../../../../../network/messages/payments/IntermediateNodeReservationResponseMessage.h"
#include "../../../../../network/messages/payments/ParticipantsConfigurationRequestMessage.h"
#include "../../../../../network/messages/payments/ParticipantsConfigurationMessage.h"
#include "../../../../../network/messages/payments/ParticipantsVotesMessage.h"
#include "../../../../../network/messages/payments/VotesStatusRequestMessage.hpp"
#include "../../../../../network/messages/payments/FinalPathConfigurationMessage.h"


// TODO: Add restoring of the reservations after transaction deserialization.
class BasePaymentTransaction:
    public BaseTransaction {

public:
    BasePaymentTransaction(
        const TransactionType type,
        const NodeUUID &currentNodeUUID,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        Logger *log);

    BasePaymentTransaction(
        const TransactionType type,
        const TransactionUUID &transactionUUID,
        const NodeUUID &currentNodeUUID,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        Logger *log);

    BasePaymentTransaction(
        const TransactionType type,
        BytesShared buffer,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        Logger *log);

protected:
    enum Stages {
        Coordinator_Initialisation = 1,
        Coordinator_ReceiverResourceProcessing,
        Coordinator_ReceiverResponseProcessing,
        Coordinator_AmountReservation,
        Coordinator_ShortPathAmountReservationResponseProcessing,
        Coordinator_PreviousNeighborRequestProcessing,
        Coordinator_FinalPathsConfigurationApproving,

        Receiver_CoordinatorRequestApproving,
        Receiver_AmountReservationsProcessing,

        IntermediateNode_PreviousNeighborRequestProcessing,
        IntermediateNode_CoordinatorRequestProcessing,
        IntermediateNode_NextNeighborResponseProcessing,
        IntermediateNode_ReservationProlongation,

        Common_VotesChecking,
        Common_FinalPathConfigurationChecking,
        Common_FinalPathsConfigurationChecking,
        Common_Recovery
    };

    enum VotesRecoveryStages {
        Common_PrepareNodesListToCheckVotes,
        Common_CheckCoordinatorVotesStage,
        Common_CheckIntermediateNodeVotesStage,
    };

protected:
    // TODO: move it into separate *.h file.
    typedef uint16_t PathUUID;

    // Stages handlers
    virtual TransactionResult::SharedConst runVotesCheckingStage();
    virtual TransactionResult::SharedConst runVotesConsistencyCheckingStage();
    virtual TransactionResult::SharedConst runFinalPathsConfigurationProcessingStage();
    virtual TransactionResult::SharedConst runFinalPathConfigurationProcessingStage();

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
        const uint16_t totalParticipantsCount) const;

    uint32_t maxCoordinatorResponseTimeout() const;

    const bool contextIsValid(
        Message::MessageType messageType) const;

    const bool positiveVoteIsPresent (
        const ParticipantsVotesMessage::ConstShared kMessage) const;

    void propagateVotesMessageToAllParticipants (
        const ParticipantsVotesMessage::Shared kMessage) const;

protected:
    // Specifies how long node must wait for the response from the remote node.
    // This timeout must take into account also that remote node may process other transaction,
    // and may be too busy to response.
    // (it is not only network transfer timeout).
    static const uint16_t kMaxMessageTransferLagMSec = 1500; // milliseconds

    // Specifies how long node may process transaction for some decision.
    static const uint16_t kExpectedNodeProcessingDelay = 1500; // milliseconds;

    // Specifies how long node must wait for the resources from other transaction
    static const uint16_t kMaxResourceTransferLagMSec = 2000; //

    static const auto kMaxPathLength = 7;

protected:
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;

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
