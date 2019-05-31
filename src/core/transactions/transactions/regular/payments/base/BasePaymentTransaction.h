#ifndef BASEPAYMENTTRANSACTION_H
#define BASEPAYMENTTRANSACTION_H


#include "../../../base/BaseTransaction.h"

#include "../../../../../contractors/ContractorsManager.h"
#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../io/storage/StorageHandler.h"
#include "../../../../../topology/cache/TopologyCacheManager.h"
#include "../../../../../topology/cache/MaxFlowCacheManager.h"
#include "../../../../../resources/manager/ResourcesManager.h"
#include "../../../../../resources/resources/BlockNumberRecourse.h"

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
#include "../../../../../network/messages/payments/TTLProlongationRequestMessage.h"
#include "../../../../../network/messages/payments/TTLProlongationResponseMessage.h"
#include "../../../../../network/messages/payments/FinalAmountsConfigurationMessage.h"
#include "../../../../../network/messages/payments/FinalAmountsConfigurationResponseMessage.h"
#include "../../../../../network/messages/payments/TransactionPublicKeyHashMessage.h"
#include "../../../../../network/messages/payments/ParticipantsPublicKeysMessage.h"
#include "../../../../../network/messages/payments/ParticipantVoteMessage.h"
#include "../../../../../observing/messages/ObservingClaimAppendRequestMessage.h"

#include "../../../../../subsystems_controller/SubsystemsController.h"

#include <unordered_set>

using namespace crypto;
namespace signals = boost::signals2;

class BasePaymentTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<BasePaymentTransaction> Shared;

public:
    typedef signals::signal<void(set<ContractorID>&, const SerializedEquivalent)> BuildCycleThreeNodesSignal;
    typedef signals::signal<void(set<ContractorID>&, const SerializedEquivalent)> BuildCycleFourNodesSignal;
    typedef signals::signal<void(ObservingClaimAppendRequestMessage::Shared)> ObservingClaimSignal;
    typedef signals::signal<void(const TransactionUUID&, BlockNumber)> TransactionCommittedObservingSignal;

public:
    BasePaymentTransaction(
        const TransactionType type,
        const SerializedEquivalent equivalent,
        bool iAmGateway,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        ResourcesManager *resourcesManager,
        Keystore *keystore,
        Logger &log,
        SubsystemsController *subsystemsController);

    BasePaymentTransaction(
        const TransactionType type,
        const TransactionUUID &transactionUUID,
        const SerializedEquivalent equivalent,
        bool iAmGateway,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        ResourcesManager *resourcesManager,
        Keystore *keystore,
        Logger &log,
        SubsystemsController *subsystemsController);

    BasePaymentTransaction(
        BytesShared buffer,
        bool iAmGateway,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        ResourcesManager *resourcesManager,
        Keystore *keystore,
        Logger &log,
        SubsystemsController *subsystemsController);

    virtual pair<BytesShared, size_t> serializeToBytes() const;

public:
    /**
     * @return coordinator UUID of current transaction
     * used in CyclesManager for resolving cycle closing conflicts
     * and in transaction scheduler
     */
    virtual BaseAddress::Shared coordinatorAddress() const;

    /**
     * @return length of cycle which is closing by current transaction
     * used in CyclesManager for resolving cycle closing conflicts
     */
    virtual const SerializedPathLengthSize cycleLength() const;

    /**
     * @return if payment transaction on Common_VotesChecking stage
     * used in CyclesManager for resolving cycle closing conflicts
     */
    bool isVotingStage() const;

    void setTransactionState(
        BasePaymentTransaction::SerializedStep transactionStage);

    void setObservingParticipantsSignatures(
        map<PaymentNodeID, lamport::Signature::Shared> participantsSignatures);

public:
    enum Stages {
        Coordinator_Initialization = 1,
        Coordinator_ReceiverResourceProcessing,
        Coordinator_ReceiverResponseProcessing,
        Coordinator_AmountReservation,
        Coordinator_ShortPathAmountReservationResponseProcessing,
        Coordinator_PreviousNeighborRequestProcessing,
        Coordinator_FinalAmountsConfigurationConfirmation,

        Receiver_CoordinatorRequestApproving,
        Receiver_AmountReservationsProcessing,

        IntermediateNode_PreviousNeighborRequestProcessing,
        IntermediateNode_CoordinatorRequestProcessing,
        IntermediateNode_NextNeighborResponseProcessing,
        IntermediateNode_ReservationProlongation,

        Cycles_WaitForOutgoingAmountReleasing,
        Cycles_WaitForIncomingAmountReleasing,

        Common_ObservingBlockNumberProcessing,
        Common_Voting,
        Common_VotesChecking,
        Common_FinalPathConfigurationChecking,
        Common_Recovery,
        Common_Observing,
        Common_ObservingReject,

        Common_RollbackByOtherTransaction,
        Common_Uncertain
    };

    enum VotesRecoveryStages {
        Common_PrepareNodesListToCheckVotes,
        Common_CheckCoordinatorVotesStage,
        Common_CheckIntermediateNodeVotesStage
    };

protected:
    TransactionResult::SharedConst runVotesCheckingStage();

    /**
     * reaction on receiving participants votes message with result of voting
     * on this stage node can commit transaction or reject it
     */
    virtual TransactionResult::SharedConst runVotesConsistencyCheckingStage();

    TransactionResult::SharedConst processParticipantsVotesMessage();

    // approving of transaction
    virtual TransactionResult::SharedConst approve();
    // recovering of transaction
    virtual TransactionResult::SharedConst recover(
        const char *message);
    // rejecting of transaction
    virtual TransactionResult::SharedConst reject(
        const char *message);

    /**
     * starts recovery process
     */
    TransactionResult::SharedConst runVotesRecoveryParentStage();

    /**
     * send message to specified node to get result of recovered transaction
     * @param contractorUUID node to which message will be sent
     */
    TransactionResult::SharedConst sendVotesRequestMessageAndWaitForResponse(
        Contractor::Shared contractor);

    /**
    * process next node in participants votes message during recovery stage
    * @return result of transaction
    */
    TransactionResult::SharedConst processNextNodeToCheckVotes();

    /**
     * prepare list of nodes which will be asked about result of recovered transaction
     */
    TransactionResult::SharedConst runPrepareListNodesToCheckNodes();

    /**
     * process response of coordinator node with result of recovered transaction
     */
    TransactionResult::SharedConst runCheckCoordinatorVotesStage();

    /**
     * process response of intermediate node with result of recovered transaction
     */
    TransactionResult::SharedConst runCheckIntermediateNodeVotesStage();

    TransactionResult::SharedConst runObservingStage();

    TransactionResult::SharedConst runObservingResultStage();

    TransactionResult::SharedConst runObservingRejectTransaction();

protected:
    /**
     * reserving outgoing amount to node on specified path
     * @param neighborNode node to which outgoing amount will be reserved
     * @param amount amount which will be reserved
     * @param pathID id of path on which amount will be reserved
     * @return true if the reservation was successful, false otherwise
     */
    const bool reserveOutgoingAmount(
        ContractorID neighborNode,
        const TrustLineAmount& amount,
        const PathID &pathID);

    /**
     * reserving incoming amount from node on specified path
     * @param neighborNode node from which incoming amount will be reserved
     * @param amount amount which will be reserved
     * @param pathID id of path on which amount will be reserved
     * @return true if the reservation was successful, false otherwise
     */
    const bool reserveIncomingAmount(
        ContractorID neighborNode,
        const TrustLineAmount& amount,
        const PathID &pathID);

    /**
     * find reservation on AmountReservationsHandler and put it into transaction reservations copy
     * used on transaction deserializing during observing
     * @param neighborNode reservation
     * @return true if reservation exists and was copied
     */
    const bool copyReservationFromGlobalReservations(
        ContractorID neighborNode,
        const TrustLineAmount& amount,
        AmountReservation::ReservationDirection reservationDirection,
        const PathID &pathID);

    /**
     * reduction amount reservation to node on specified path
     * @param kContractor node with which incoming amount will be reduced
     * @param kReservation pointer to reservation which will be reduced
     * @param kNewAmount new amount of reservation
     * @param pathID id of path on which amount will be reduced
     * @return true if the reservation was reduced successfully, false otherwise
     */
    const bool shortageReservation(
        ContractorID kContractor,
        const AmountReservation::ConstShared kReservation,
        const TrustLineAmount &kNewAmount,
        const PathID &pathID);

    /**
     * save participants votes message into storage
     * @param ioTransaction pointer on database transaction
     */
    void saveVotes(
        IOTransaction::Shared ioTransaction);

    /**
     * commit current transaction by using all reservations
     * @param ioTransaction pointer on database transaction
     */
    void commit(
        IOTransaction::Shared ioTransaction);

    /**
     * rollback current transaction by dropping all reservations
     */
    void rollBack();

    /**
     * dropping reservations on specified path
     * @param pathID id of path on which reservations will be dropped
     */
    void rollBack(
        const PathID &pathID);

    void removeAllDataFromStorageConcerningTransaction(
        IOTransaction::Shared ioTransaction = nullptr);

    /**
     * @param totalHopsCount count of sending messages
     * @return max time in milliseconds of waiting response
     */
    uint32_t maxNetworkDelay (
        const uint16_t totalHopsCount) const;

    /**
     * check messages context
     * @param messageType type of message which should be on top of message context
     * @param showErrorMessage indicates if show error message if context is not valid
     * @return true next message in messages context is valid
     */
    const bool contextIsValid(
        Message::MessageType messageType,
        bool showErrorMessage = true) const;

    const bool resourceIsValid(
        BaseResource::ResourceType resourceType) const;

    /**
     * drop all reservations of current node on specified path
     * @param pathID id of path on which reservations will be dropped
     */
    void dropNodeReservationsOnPath(
        PathID pathID);

    /**
     * run signal for building cycles on three nodes
     */
    void runThreeNodesCyclesTransactions();

    /**
     * run signal for building cycles on four nodes
     */
    void runFourNodesCyclesTransactions();

    // Updates all reservations according to finalAmounts
    // if some reservations will be found, pathIDs of which are absent in finalAmounts, returns false,
    // otherwise returns true
    bool updateReservations(
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmounts);

    // Returns reservation pathID, which was updated, if reservation was dropped, returns 0
    /**
     * update reservations of current node to specified node on specified path
     * @param contractorID node with which reservation will be updated
     * @param pathIDAndReservation pair of path id and pointer to reservations which will be updated
     * @param finalAmounts vector of currently final amounts on all paths
     * @return path id of reservation if it was updated or 0 if reservation was dropped
     */
    PathID updateReservation(
        ContractorID contractorID,
        pair<PathID, AmountReservation::ConstShared> &pathIDAndReservation,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmounts);

    /**
     * @return bytes cont of serialized reservation
     */
    size_t reservationsSizeInBytes() const;

    /**
     * @param reservationDirection direction (outgoing or incoming) total amount of which will be returned
     * @return total reserved amount of current node on specified direction
     */
    const TrustLineAmount totalReservedAmount(
        AmountReservation::ReservationDirection reservationDirection) const;

    /**
     * save result of payment transaction on database, implements by all transactions in different ways
     * @param ioTransaction pointer on database transaction
     */
    virtual void savePaymentOperationIntoHistory(
        IOTransaction::Shared ioTransaction) = 0;

    /**
     * check if reservations on current node are valid before committing
     * implements by all transactions in different ways
     * (all outgoing amounts on paths have equals incoming amounts)
     * @return true if reservations are valid
     */
    virtual bool checkReservationsDirections() const = 0;

    pair<BytesShared, size_t> getSerializedReceipt(
        ContractorID source,
        ContractorID target,
        const TrustLineAmount &amount,
        bool isSource);

    bool checkAllNeighborsWithReservationsAreInFinalParticipantsList();

    bool checkAllPublicKeyHashesProperly();

    const TrustLineAmount totalReservedIncomingAmountToNode(
        ContractorID contractorID);

    bool checkPublicKeysAppropriate();

    pair<BytesShared, size_t> getSerializedParticipantsVotesData(
        Contractor::Shared contractor);

    bool checkSignaturesAppropriate();

    bool checkMaxClaimingBlockNumber(
        BlockNumber maxClaimingBlockNumberOnOwnSide);

protected:
    // Specifies how long node must wait for the response from the remote node.
    // This timeout must take into account also that remote node may process other transaction,
    // and may be too busy to response.
    // (it is not only network transfer timeout).
    // it used on calculating maxNetworkDelay
    static const uint16_t kMaxMessageTransferLagMSec = 1500; // milliseconds

    // Specifies how long node must wait for the resources from other transaction
    static const uint16_t kMaxResourceTransferLagMSec = 2000; //

    // max length of transaction path
    static const auto kMaxPathLength = 7;

    static const uint32_t kWaitMillisecondsToTryRecoverAgain = 30000;
    static const uint8_t kMaxRecoveryAttempts = 3;

    // todo : make static
    const PaymentNodeID kCoordinatorPaymentNodeID = 0;

    static const uint64_t kCountBlocksForClaiming = 20;
    static const uint64_t kAllowableBlockNumberDifference = 2;

    static const uint32_t kMaxHoursTransactionDuration = 0;
    static const uint32_t kMaxMinutesTransactionDuration = 0;
    static const uint32_t kMaxSecondsTransactionDuration = 30;

    static Duration& kMaxTransactionDuration() {
        static auto duration = Duration(
            kMaxHoursTransactionDuration,
            kMaxMinutesTransactionDuration,
            kMaxSecondsTransactionDuration);
        return duration;
    }

public:
    // signal for launching transaction of building cycles on three nodes
    mutable BuildCycleThreeNodesSignal mBuildCycleThreeNodesSignal;

    // signal for launching transaction of building cycles on four nodes
    mutable BuildCycleFourNodesSignal mBuildCycleFourNodesSignal;

    mutable ObservingClaimSignal observingClaimSignal;

    mutable TransactionCommittedObservingSignal mTransactionCommittedObservingSignal;

protected:
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;
    ResourcesManager *mResourcesManager;
    Keystore *mKeysStore;
    SubsystemsController *mSubsystemsController;

    bool mTTLRequestWasSend;

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

    map<ContractorID, vector<pair<PathID, AmountReservation::ConstShared>>> mReservations;

    // Nodes which with current node will be trying to close cycle
    set<ContractorID> mContractorsForCycles;
    bool mIAmGateway;

    // Votes recovery
    uint8_t mVotesRecoveryStep;
    vector<Contractor::Shared> mNodesToCheckVotes;
    Contractor::Shared mCurrentNodeToCheckVotes;
    uint8_t mCountRecoveryAttempts;

    // this amount used for saving in payment history
    TrustLineAmount mCommittedAmount;

    // ids of nodes inside payment transaction
    map<PaymentNodeID, Contractor::Shared> mPaymentParticipants;
    map<string, PaymentNodeID> mPaymentNodesIds;
    map<string, pair<PaymentNodeID, lamport::KeyHash::Shared>> mParticipantsPublicKeysHashes;
    map<PaymentNodeID, lamport::PublicKey::Shared> mParticipantsPublicKeys;
    map<PaymentNodeID, lamport::Signature::Shared> mParticipantsSignatures;

    // this fields are used by coordinators on final amount configuration clarification
    bool mAllNodesSentConfirmationOnFinalAmountsConfiguration;
    bool mAllNeighborsSentFinalReservations;

    lamport::PublicKey::Shared mPublicKey;

    BlockNumber mMaximalClaimingBlockNumber;
    bool mBlockNumberObtainingInProcess;

    string mPayload;
};

#endif // BASEPAYMENTTRANSACTION_H
