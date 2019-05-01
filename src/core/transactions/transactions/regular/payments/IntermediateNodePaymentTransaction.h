#ifndef INTERMEDIATENODEPAYMENTTRANSACTION_H
#define INTERMEDIATENODEPAYMENTTRANSACTION_H

#include "base/BasePaymentTransaction.h"

class IntermediateNodePaymentTransaction:
    public BasePaymentTransaction {

public:
    typedef shared_ptr<IntermediateNodePaymentTransaction> Shared;
    typedef shared_ptr<const IntermediateNodePaymentTransaction> ConstShared;

public:
    IntermediateNodePaymentTransaction(
        IntermediateNodeReservationRequestMessage::ConstShared message,
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

    IntermediateNodePaymentTransaction(
        BytesShared buffer,
        bool iAmGateway,
        ContractorsManager *contractorsManager,
        TrustLinesManager* trustLines,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        ResourcesManager *resourcesManager,
        Keystore *keystore,
        Logger &log,
        SubsystemsController *subsystemsController);

    TransactionResult::SharedConst run() override;

    /**
     * @return Address of coordinator node of current transaction
     */
    BaseAddress::Shared coordinatorAddress() const override;

protected:
    /**
     * reaction on reservation request message from previous node on processed path
     * try reserve requested incoming amount and send reservation response
     */
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStage();

    /**
     * reaction on coordinator further reservation request message
     * try reserve requested outgoing amount to next node on processed path
     * and send reservation request message to this node
     */
    TransactionResult::SharedConst runCoordinatorRequestProcessingStage();

    /**
     * reaction on next node reservation response message
     * send reservation response to coordinator
     */
    TransactionResult::SharedConst runNextNeighborResponseProcessingStage();

    /**
     * reaction on message with final amount configuration on processed path from coordinator
     * update all reservations on this path according to received final configuration
     */
    TransactionResult::SharedConst runFinalPathConfigurationProcessingStage();

    /**
     * reaction on any message and run appropriate method
     * if no message received then send message (TTL)
     * to coordinator with request if transaction is still alive
     */
    TransactionResult::SharedConst runReservationProlongationStage();

    TransactionResult::SharedConst runFinalAmountsConfigurationConfirmation();

    /**
     * reaction on message with final amounts configuration (on all paths) from coordinator
     * update all reservations according to received final configuration
     * and send response if all reservations was successfully updated
     */
    TransactionResult::SharedConst runFinalReservationsCoordinatorConfirmation();

    TransactionResult::SharedConst runFinalReservationsNeighborConfirmation();

    TransactionResult::SharedConst runCheckObservingBlockNumber();

    /**
     * reaction on receiving participants votes message firstly
     * add own vote to message and send it to next participant
     * if no message received then send message (TTL)
     * to coordinator with request if transaction is still alive
     */
    TransactionResult::SharedConst runVotesCheckingStageWithCoordinatorClarification();

protected:
    // Intermediate node must launch closing cycles 3 and 4 transactions.
    // Therefore this methods are overridden.
    TransactionResult::SharedConst approve() override;

    /**
     * reduce amount reservations (incoming and outgoing) on specified path
     * @param pathID id of path on which amount will be reduced
     * @param amount new amount of reservations
     */
    void shortageReservationsOnPath(
        const PathID pathID,
        const TrustLineAmount &amount);

    /**
     * save result of payment transaction on database
     * @param ioTransaction pointer on database transaction
     */
    void savePaymentOperationIntoHistory(
        IOTransaction::Shared ioTransaction) override;

    /**
     * check if reservations on current node are valid before committing
     * (all outgoing amounts on paths have equals incoming amounts)
     * @return true if reservations are valid
     */
    bool checkReservationsDirections() const override;

    TransactionResult::SharedConst sendErrorMessageOnCoordinatorRequest(
        ResponseMessage::OperationState errorState);

    TransactionResult::SharedConst sendErrorMessageOnPreviousNodeRequest(
        BaseAddress::Shared previousNode,
        PathID pathID,
        ResponseMessage::OperationState errorState);

    TransactionResult::SharedConst sendErrorMessageOnNextNodeResponse(
        ResponseMessage::OperationState errorState);

    void sendErrorMessageOnFinalAmountsConfiguration();

    const string logHeader() const override;

protected:
    // message on which current transaction was started
    IntermediateNodeReservationRequestMessage::ConstShared mMessage;

    TrustLineAmount mLastReservedAmount;
    Contractor::Shared mCoordinator;
    // id of path, which was processed last
    PathID mLastProcessedPath;
};


#endif // INTERMEDIATENODEPAYMENTTRANSACTION_H
