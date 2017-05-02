#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H


#include "base/BasePaymentTransaction.h"
#include "base/PathStats.h"
#include "../../find_path/FindPathTransaction.h"
#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"

#include "../../../../resources/manager/ResourcesManager.h"

#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <unordered_set>

/**
 * TODO: Implement intermedaite reservations shortage for the big transactions.
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
        ResourcesManager *resourcesManager,
        Logger *log)
        noexcept;

    CoordinatorPaymentTransaction(
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
    // Stages handlers
    // TODO: Add throws specififcations
    TransactionResult::SharedConst runPaymentInitialisationStage ();
    TransactionResult::SharedConst runReceiverResourceProcessingStage();
    TransactionResult::SharedConst runReceiverResponseProcessingStage ();
    TransactionResult::SharedConst runAmountReservationStage ();
    TransactionResult::SharedConst runDirectAmountReservationResponseProcessingStage ();
    TransactionResult::SharedConst propagateVotesListAndWaitForConfigurationRequests ();
    TransactionResult::SharedConst runFinalParticipantsRequestsProcessingStage ();

protected:
    // Coordinator must return command result on transaction finishing.
    // Therefore this methods are overriden.
    TransactionResult::SharedConst approve();
    TransactionResult::SharedConst recover(
        const char *message = nullptr);
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
    void addPathForFurtherProcessing(
        Path::ConstShared path);

    void initAmountsReservationOnNextPath();

    void switchToNextPath();

    PathStats* currentAmountReservationPathStats();

    TransactionResult::SharedConst tryProcessNextPath();

    TransactionResult::SharedConst tryReserveAmountDirectlyOnReceiver (
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

    TrustLineAmount totalReservedByAllPaths() const;

protected:
    const string logHeader() const;

    void deserializeFromBytes(
        BytesShared buffer);

    bool isPathValid(
        Path::Shared path) const;

protected:
    CreditUsageCommand::Shared mCommand;

    // Contains special stats data, such as current msx flow,
    // for all paths involved into the transaction.
    unordered_map<PathUUID, unique_ptr<PathStats>, boost::hash<boost::uuids::uuid>> mPathsStats;

    // Used in amount reservations stage.
    // Contains identifier of the path,
    // that was processed last, and potenially,
    // is waiting for request appriving.
    PathUUID mCurrentAmountReservingPathIdentifier;

    unordered_map<PathUUID, unique_ptr<PathStats>, boost::hash<boost::uuids::uuid>>::const_iterator mCurrentAmountReservingPathIterator;

    // Reservation stage contains it's own internal steps counter.
    byte mReservationsStage;

    // Contains nodes that has been requrested final paths configuration.
    // for more details, see TODO
    unordered_set<NodeUUID> mNodesRequestedFinalConfiguration;

    /*
     * If true - then it means that direct path betweeen coordinator and receiver has been already processed.
     * Otherwise is set to the false (by default).
     *
     * Only one direct path may occure due to one payment operation.
     * In case if several direct paths occurs - than it seems that paths collection is broken.
     */
    bool mDirectPathIsAllreadyProcessed;

    ResourcesManager *mResourcesManager;
};
#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
