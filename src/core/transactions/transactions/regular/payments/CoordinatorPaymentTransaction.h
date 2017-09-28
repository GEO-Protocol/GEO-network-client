#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H


#include "base/BasePaymentTransaction.h"
#include "base/PathStats.h"
#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"

#include "../../../../resources/manager/ResourcesManager.h"
#include "../../../../paths/PathsManager.h"
#include "../../../../resources/resources/PathsResource.h"

#include "../../../../io/storage/record/payment/PaymentRecord.h"

#include "../../../../common/exceptions/CallChainBreakException.h"

#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <chrono>
#include <thread>

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
        const NodeUUID &kCurrentNodeUUID,
        const CreditUsageCommand::Shared kCommand,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        ResourcesManager *resourcesManager,
        PathsManager *pathsManager,
        Logger &log,
        SubsystemsController *subsystemsController)
        noexcept;

    CoordinatorPaymentTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        ResourcesManager *resourcesManager,
        PathsManager *pathsManager,
        Logger &log,
        SubsystemsController *subsystemsController)
        throw (bad_alloc);

    TransactionResult::SharedConst run()
        noexcept;

    const NodeUUID& coordinatorUUID() const;

protected:
    // Stages handlers
    // TODO: Add throws specififcations
    TransactionResult::SharedConst runPaymentInitialisationStage ();
    TransactionResult::SharedConst runReceiverResourceProcessingStage();
    TransactionResult::SharedConst runReceiverResponseProcessingStage ();
    TransactionResult::SharedConst runAmountReservationStage ();
    TransactionResult::SharedConst runDirectAmountReservationResponseProcessingStage ();
    TransactionResult::SharedConst runFinalAmountsConfigurationConfirmation();
    TransactionResult::SharedConst runVotesConsistencyCheckingStage();
    TransactionResult::SharedConst runTTLTransactionResponse();

protected:
    // Coordinator must return command result on transaction finishing.
    // Therefore this methods are overridden.
    TransactionResult::SharedConst approve();
    TransactionResult::SharedConst reject(
        const char *message = nullptr);

protected:
    // Results handlers
    TransactionResult::SharedConst resultOK();
    TransactionResult::SharedConst resultForbiddenRun();
    TransactionResult::SharedConst resultNoPathsError();
    TransactionResult::SharedConst resultProtocolError();
    TransactionResult::SharedConst resultNoResponseError();
    TransactionResult::SharedConst resultInsufficientFundsError();
    TransactionResult::SharedConst resultNoConsensusError();
    TransactionResult::SharedConst resultUnexpectedError();

protected:
    TransactionResult::SharedConst propagateVotesListAndWaitForVotingResult();

    void addPathForFurtherProcessing(
        Path::ConstShared path);

    void initAmountsReservationOnNextPath();

    void switchToNextPath();

    PathStats* currentAmountReservationPathStats();

    TransactionResult::SharedConst tryProcessNextPath();

    TransactionResult::SharedConst tryReserveAmountDirectlyOnReceiver (
        const PathID pathID,
        PathStats *pathStats);

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

    TransactionResult::SharedConst sendFinalAmountsConfigurationToAllParticipants();

    // add final path configuration to mNodesFinalAmountsConfiguration for all path nodes
    void addFinalConfigurationOnPath(
        PathID pathID,
        PathStats* pathStats);

    void shortageReservationsOnPath(
        const NodeUUID& neighborUUID,
        const PathID pathID,
        const TrustLineAmount &amount);

    // This method drops reservations on given path
    // and inform intermediate nodes about cancelling reservations on this path.
    // sendToLastProcessedNode indicates if message with 0 amount
    // will be send to current node on path.
    void dropReservationsOnPath(
        PathStats *pathStats,
        PathID pathID,
        bool sendToLastProcessedNode = false);

    [[deprecated("Use BasePaymentTransaction::totalReservedAmount() instead")]]
    TrustLineAmount totalReservedByAllPaths() const;

    void informAllNodesAboutTransactionFinish();

    void savePaymentOperationIntoHistory(
        IOTransaction::Shared ioTransaction);

    bool checkReservationsDirections() const;

protected:
    const string logHeader() const;

    bool isPathValid(
        Path::Shared path) const;

    void buildPathsAgain();

protected:
    // todo discuss this parameter
    static const uint8_t kMaxReceiverInaccessible = 5;

protected:
    CreditUsageCommand::Shared mCommand;

    // Contains special stats data, such as current msx flow,
    // for all paths involved into the transaction.
    unordered_map<PathID, unique_ptr<PathStats>> mPathsStats;

    // Used in amount reservations stage.
    // Contains identifier of the path,
    // that was processed last, and potentially,
    // is waiting for request approving.
    PathID mCurrentAmountReservingPathIdentifier;

    vector<PathID> mPathIDs;

    // Reservation stage contains it's own internal steps counter.
    byte mReservationsStage;

    /*
     * If true - then it means that direct path between coordinator and receiver has been already processed.
     * Otherwise is set to the false (by default).
     *
     * Only one direct path may occure due to one payment operation.
     * In case if several direct paths occurs - than it seems that paths collection is broken.
     */
    bool mDirectPathIsAlreadyProcessed;

    // Contains all nodes final amount configuration on all transaction paths
    map<NodeUUID, vector<pair<PathID, ConstSharedTrustLineAmount>>> mNodesFinalAmountsConfiguration;

    // Contains flags if nodes confirmed final amounts configuration,
    // before voting stage
    unordered_map<NodeUUID, bool, boost::hash<boost::uuids::uuid>> mFinalAmountNodesConfirmation;

    ResourcesManager *mResourcesManager;
    PathsManager *mPathsManager;
    set<NodeUUID> mInaccessibleNodes;
    size_t mPreviousInaccessibleNodesCount;
    vector<pair<NodeUUID, NodeUUID>> mRejectedTrustLines;
    size_t mPreviousRejectedTrustLinesCount;

    uint8_t mCountReceiverInaccessible;
};
#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
