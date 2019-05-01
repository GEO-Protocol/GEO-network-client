#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H


#include "base/BasePaymentTransaction.h"
#include "../../../../paths/PathsManager.h"
#include "../../../../interface/events_interface/interface/EventsInterface.h"
#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"
#include "../../../../resources/resources/PathsResource.h"
#include "base/PathStats.h"
#include "../../../../common/exceptions/CallChainBreakException.h"

#include <unordered_map>

/**
 * TODO: Implement intermediate reservations shortage for the big transactions.
 * It is makes sense to implement additional reservations shortage process,
 * that would free parts of reserved capabilities in parallel with amounts reservation stages.
 */
class CoordinatorPaymentTransaction:
    public BasePaymentTransaction {

public:
    typedef shared_ptr<CoordinatorPaymentTransaction> Shared;
    typedef shared_ptr<const CoordinatorPaymentTransaction> ConstShared;

public:
    CoordinatorPaymentTransaction(
        const CreditUsageCommand::Shared command,
        bool iAmGateway,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        ResourcesManager *resourcesManager,
        PathsManager *pathsManager,
        Keystore *keystore,
        bool isPaymentTransactionsAllowedDueToObserving,
        EventsInterface *eventsInterface,
        Logger &log,
        SubsystemsController *subsystemsController);

    TransactionResult::SharedConst run() override;

    const CommandUUID& commandUUID() const;

protected:
    // Stages handlers
    // TODO: Add throws specifications

    /**
     * process initialisation payment transaction
     * check conditions if transaction can be run
     * send request for building payment paths
     */
    TransactionResult::SharedConst runPaymentInitializationStage();

    /**
     * process the result of building paths
     * send request to Receiver for initialization payment transaction on it
     */
    TransactionResult::SharedConst runPathsResourceProcessingStage();

    /**
     * process response initialization from Receiver
     */
    TransactionResult::SharedConst runReceiverResponseProcessingStage();

    /**
     * process the reservation of transaction amount on built paths
     */
    TransactionResult::SharedConst runAmountReservationStage();

    /**
     * reaction on request of reserve amount on direct way to Receiver
     */
    TransactionResult::SharedConst runDirectAmountReservationResponseProcessingStage();

    /**
     * reaction on messages with approving or not of final amounts configuration from all participants
     */
    TransactionResult::SharedConst runFinalAmountsConfigurationConfirmation();

    /**
     * reaction on receiving participants votes message with result of voting
     * on this stage node can commit transaction or reject it
     * and send result to all participants
     */
    TransactionResult::SharedConst runVotesConsistencyCheckingStage() override;

    /*
     * reaction on message from some node if transaction is still alive
     * send message to requester with instruction "continue transaction" or "finish transaction"
     */
    TransactionResult::SharedConst runTTLTransactionResponse();

protected:
    // Coordinator must return command result on transaction finishing.
    // Therefore this methods are overridden.
    TransactionResult::SharedConst approve() override;

    TransactionResult::SharedConst reject(
        const char *message) override;

protected:
    // Results handlers
    TransactionResult::SharedConst resultOK();
    TransactionResult::SharedConst resultForbiddenRun();
    TransactionResult::SharedConst resultForbiddenRunDueObserving();
    TransactionResult::SharedConst resultNoPathsError();
    TransactionResult::SharedConst resultProtocolError();
    TransactionResult::SharedConst resultNoResponseError();
    TransactionResult::SharedConst resultInsufficientFundsError();
    TransactionResult::SharedConst resultNoConsensusError();
    TransactionResult::SharedConst resultUnexpectedError();

protected:
    TransactionResult::SharedConst propagateVotesListAndWaitForVotingResult();

    /**
     * add built path to mPathsStats for further processing on amount reservation stage
     * @param path built path from resources which will be added to mPathsStats
     */
    void addPathForFurtherProcessing(
        Path::Shared path);

    /**
     * init field mCurrentAmountReservingPathIdentifier for starting work with mPathStats
     */
    void initAmountsReservationOnNextPath();

    /**
     * switch to processing next path from mPathStats
     */
    void switchToNextPath();

    /**
     * @return current processing path
     */
    PathStats* currentAmountReservationPathStats();

    /**
     * try switch to next path
     * if paths is over, try rebuild new paths and switch on new path
     * in case if no new path build, returns resultInsufficientFundsError
     */
    TransactionResult::SharedConst tryProcessNextPath();

    /**
     * try reserve available amount on direct path to Receiver
     * and send reservation request to it
     * @param pathID id of path on which amount reserved
     * @param pathStats path in which amount reserved
     */
    TransactionResult::SharedConst tryReserveAmountDirectlyOnReceiver (
        const PathID pathID,
        PathStats *pathStats);

    /**
     * try reserve available amount on next node on path
     * @param pathStats path on which trying reserve
     */
    TransactionResult::SharedConst tryReserveNextIntermediateNodeAmount (
        PathStats *pathStats);

    /**
     * send reservation request to neighbor on specified path
     * @param neighbor neighbor of current node on which reservation request will be sent
     * @param pathStats path on which thr reservation is made
     */
    TransactionResult::SharedConst askNeighborToReserveAmount(
        BaseAddress::Shared neighbor,
        PathStats *pathStats);

    /**
     * reaction on reservation response from neighbor
     */
    TransactionResult::SharedConst processNeighborAmountReservationResponse();

    /**
     * send further reservation request to neighbor (neighbor should reserve amount to his neighbor)
     * @param neighbor neighbor of current node on which further reservation request will be sent
     * @param pathStats path on which thr reservation is made
     */
    TransactionResult::SharedConst askNeighborToApproveFurtherNodeReservation(
        BaseAddress::Shared neighbor,
        PathStats *pathStats);

    /**
     * reaction on further reservation response from neighbor
     */
    TransactionResult::SharedConst processNeighborFurtherReservationResponse();

    /**
     * send further reservation request to remote intermediate node (node should reserve amount to his neighbor)
     * @param pathStats path on which thr reservation is made
     * @param remoteNode node to which current node send request
     * @param remoteNodePosition position of remote node in pathStats
     * @param nextNodeAfterRemote neighbor of remote node to which it should reserve available amount
     */
    TransactionResult::SharedConst askRemoteNodeToApproveReservation(
        PathStats *pathStats,
        BaseAddress::Shared remoteNode,
        const byte remoteNodePosition,
        BaseAddress::Shared nextNodeAfterRemote);

    /**
     * reaction on further reservation response from remote node
     */
    TransactionResult::SharedConst processRemoteNodeResponse();

    /**
     * send messages to all transaction participants with their final amount configuration
     */
    TransactionResult::SharedConst sendFinalAmountsConfigurationToAllParticipants();

    // add final path configuration to mNodesFinalAmountsConfiguration for all path nodes
    // todo : refactor without pathID and pathStats, use inside mCurrentAmountReservingPathIdentifier
    void addFinalConfigurationOnPath(
        PathID pathID,
        PathStats* pathStats);

    /**
     * reduce amount reservation to node on specified path
     * @param neighborUUID neighbor node with which reservation will be reduce
     * @param pathID id of path on which amount will be reduced
     * @param amount new amount of reservation
     */
    // todo : refactor without neighborID and pathID, use inside mCurrentAmountReservingPathIdentifier
    void shortageReservationsOnPath(
        ContractorID neighborID,
        const PathID pathID,
        const TrustLineAmount &amount);

    /**
     * drop reservation to neighbor on specified path
     * and inform intermediate nodes about cancelling reservations on this path.
     * @param pathStats path on which reservations will be dropped
     * @param pathID id of path on which reservations will be dropped
     * @param sendToLastProcessedNode indicates if last processed node will be informed
     */
    // todo : refactor without pathStats and pathID, use inside mCurrentAmountReservingPathIdentifier
    void dropReservationsOnPath(
        PathStats *pathStats,
        PathID pathID,
        bool sendToLastProcessedNode = false);

    /**
    * send message with final amounts configuration (final amount which should be reserved)
    * on specified path to all participants of transaction
    * @param pathStats path with nodes status, final amount configuration of which will be sent
    * @param pathID id of path, final amount configuration of which will be sent
    * @param finalPathAmount final amount which should be reserved on specified path
    */
// todo : refactor without pathStats and pathID, use inside mCurrentAmountReservingPathIdentifier
    void sendFinalPathConfiguration(
        PathStats* pathStats,
        PathID pathID,
        const TrustLineAmount &finalPathAmount);

    /**
     * send messages to all transaction participants with instruction "finish transaction"
     */
    void informAllNodesAboutTransactionFinish();

    /**
     * save result of payment transaction on database
     * @param ioTransaction pointer on database transaction
     */
    void savePaymentOperationIntoHistory(
        IOTransaction::Shared ioTransaction) override;

    /**
     * check if reservations on current node are valid before committing
     * (all reservations should be outgoing)
     * @return true if reservations are valid
     */
    bool checkReservationsDirections() const override;

protected:
    const string logHeader() const override;

    /**
     * check if built path is valid before adding it to mPathStats
     * @param path
     * @return
     */
    bool isPathValid(
        Path::Shared path) const;

    /**
     * try build new paths if reserved amount on previously built paths is less then required
     */
    void buildPathsAgain();

protected:
    // todo discuss this parameter
    // max count failed attempts to connect with Receiver, after which transaction will be rollbacked
    static const uint8_t kMaxReceiverInaccessible = 5;

    static const uint16_t kMaxRebuildingAttemptsCount = 3;

protected:
    EventsInterface *mEventsInterface;

    // Command on which current transaction was started
    CreditUsageCommand::Shared mCommand;
    Contractor::Shared mContractor;

    // Contains special stats data, such as current max flow,
    // for all paths involved into the transaction.
    unordered_map<PathID, unique_ptr<PathStats>> mPathsStats;

    // Used in amount reservations stage.
    // Contains identifier of the path,
    // that was processed last, and potentially,
    // is waiting for request approving.
    PathID mCurrentAmountReservingPathIdentifier;

    // Contains all path ids which should be processed
    vector<PathID> mPathIDs;

    // Reservation stage contains it's own internal steps counter.
    byte mReservationsStage;

    // Contains all addresses of participants of current path.
    // Only main addresses are used for building paths.
    // During reservations all participants inform coordinator about theirs all addresses.
    vector<Contractor::Shared> mCurrentPathParticipants;

    /*
     * If true - then it means that direct path between coordinator and receiver has been already processed.
     * Otherwise is set to the false (by default).
     *
     * Only one direct path may occur due to one payment operation.
     * In case if several direct paths occurs - than it seems that paths collection is broken.
     */
    bool mDirectPathIsAlreadyProcessed;

    // Contains all nodes final amount configuration on all transaction paths
    map<string, vector<pair<PathID, ConstSharedTrustLineAmount>>> mNodesFinalAmountsConfiguration;

    PathsManager *mPathsManager;
    vector<BaseAddress::Shared> mInaccessibleNodes;
    size_t mPreviousInaccessibleNodesCount;
    vector<pair<BaseAddress::Shared, BaseAddress::Shared>> mRejectedTrustLines;
    size_t mPreviousRejectedTrustLinesCount;
    PaymentNodeID mCurrentFreePaymentID;
    uint16_t mRebuildingAttemptsCount;

    // indicates that there are TL with keys absent problem
    bool mNeighborsKeysProblem;

    // indicates that there are participants which have TL with keys absent problem
    bool mParticipantsKeysProblem;

    // count failed attempts to connect with Receiver
    uint8_t mCountReceiverInaccessible;

    bool mIsPaymentTransactionsAllowedDueToObserving;
};
#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
