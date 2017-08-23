#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H


#include "base/BasePaymentTransaction.h"
#include "base/PathStats.h"
#include "../../find_path/FindPathTransaction.h"
#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"

#include "../../../../resources/manager/ResourcesManager.h"
#include "../../../../paths/PathsManager.h"

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
    TransactionResult::SharedConst runTTLTransactionResponce();

protected:
    // Coordinator must return command result on transaction finishing.
    // Therefore this methods are overriden.
    TransactionResult::SharedConst approve();
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
    TransactionResult::SharedConst resultUnexpectedError();

protected:
    TransactionResult::SharedConst propagateVotesListAndWaitForVoutingResult();

    void addPathForFurtherProcessing(
        Path::ConstShared path);

    void initAmountsReservationOnNextPath();

    void switchToNextPath();

    PathStats* currentAmountReservationPathStats();

    TransactionResult::SharedConst tryProcessNextPath();

    TransactionResult::SharedConst tryReserveAmountDirectlyOnReceiver (
        const PathUUID pathUUID,
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
        PathUUID pathUUID,
        PathStats* pathStats);

    [[deprecated("Use BasePaymentTransaction::totalReservedAmount() instead")]]
    TrustLineAmount totalReservedByAllPaths() const;

    void informAllNodesAboutTransactionFinish();

    void savePaymentOperationIntoHistory();

    bool checkReservationsDirections() const;

    void runBuildThreeNodesCyclesSignal();

protected:
    const string logHeader() const;

    bool isPathValid(
        Path::Shared path) const;

    void buildPathsAgain();

protected:
    // todo disscuss this parameter
    static const uint8_t kMaxReceiverInaccessible = 5;

protected:
    CreditUsageCommand::Shared mCommand;

    // Contains special stats data, such as current msx flow,
    // for all paths involved into the transaction.
    unordered_map<PathUUID, unique_ptr<PathStats>> mPathsStats;

    // Used in amount reservations stage.
    // Contains identifier of the path,
    // that was processed last, and potentially,
    // is waiting for request approving.
    PathUUID mCurrentAmountReservingPathIdentifier;

    vector<PathUUID> mPathUUIDs;

    // Reservation stage contains it's own internal steps counter.
    byte mReservationsStage;

    /*
     * If true - then it means that direct path between coordinator and receiver has been already processed.
     * Otherwise is set to the false (by default).
     *
     * Only one direct path may occure due to one payment operation.
     * In case if several direct paths occurs - than it seems that paths collection is broken.
     */
    bool mDirectPathIsAllreadyProcessed;

    // Contains all nodes final amount configuration on all transaction paths
    map<NodeUUID, vector<pair<PathUUID, ConstSharedTrustLineAmount>>> mNodesFinalAmountsConfiguration;

    // Contains flags if nodes confirmed final amounts configuration,
    // before voting stage
    unordered_map<NodeUUID, bool, boost::hash<boost::uuids::uuid>> mFinalAmountNodesConfirmation;

    ResourcesManager *mResourcesManager;
    PathsManager *mPathsManager;
    set<NodeUUID> mInaccessibleNodes;

    uint8_t mCountReceiverInaccessible;
};
#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
