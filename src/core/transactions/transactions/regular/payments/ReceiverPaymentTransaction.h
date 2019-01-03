#ifndef GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H

#include "base/BasePaymentTransaction.h"

class ReceiverPaymentTransaction:
    public BasePaymentTransaction {

public:
    typedef shared_ptr<ReceiverPaymentTransaction> Shared;
    typedef shared_ptr<const ReceiverPaymentTransaction> ConstShared;

public:
    ReceiverPaymentTransaction(
        ReceiverInitPaymentRequestMessage::ConstShared message,
        bool iAmGateway,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        Keystore *keystore,
        Logger &log,
        SubsystemsController *subsystemsController);

    ReceiverPaymentTransaction(
        BytesShared buffer,
        bool iAmGateway,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        Keystore *keystore,
        Logger &log,
        SubsystemsController *subsystemsController);

    TransactionResult::SharedConst run()
        noexcept;

    /**
     * @return Address of coordinator node of current transaction
     */
    BaseAddress::Shared coordinatorAddress() const override;

protected:
    /**
     * reaction on initialization request from coordinator node,
     * check if transaction can be runned and send response message
     */
    TransactionResult::SharedConst runInitializationStage();

    /**
     * reaction on reservation request message from previous node on processed path
     * try reserve requested incoming amount and send reservation response
     */
    TransactionResult::SharedConst runAmountReservationStage();

    /**
     * reaction on response TTL message from coordinator
     * before receiving participants votes message,
     * if no message received, reject this transaction
     */
    TransactionResult::SharedConst runClarificationOfTransactionBeforeVoting();

    TransactionResult::SharedConst runFinalAmountsConfigurationConfirmation();

    TransactionResult::SharedConst runFinalReservationsCoordinatorConfirmation();

    TransactionResult::SharedConst runFinalReservationsNeighborConfirmation();

    TransactionResult::SharedConst runClarificationOfTransactionDuringFinalAmountsClarification();

    /**
     * reaction on receiving participants votes message firstly
     * add own vote to message and send it to next participant
     * if no message received then send message (TTL)
     * to coordinator with request if transaction is still alive
     */
    TransactionResult::SharedConst runVotesCheckingStageWithCoordinatorClarification();

    /**
     * reaction on response TTL message from coordinator
     * after receiving participants votes message
     * if no message received, reject this transaction
     */
    TransactionResult::SharedConst runClarificationOfTransactionDuringVoting();

protected:
    // Receiver must must save payment operation into history.
    // Therefore this methods are overridden.
    TransactionResult::SharedConst approve();

protected:
    /**
     * save result of payment transaction on database
     * @param ioTransaction pointer on database transaction
     */
    void savePaymentOperationIntoHistory(
        IOTransaction::Shared ioTransaction);

    /**
     * check if reservations on current node are valid before committing
     * (all reservations should be incoming)
     * @return true if reservations are valid
     */
    bool checkReservationsDirections() const;

    TransactionResult::SharedConst sendErrorMessageOnPreviousNodeRequest(
        BaseAddress::Shared previousNode,
        PathID pathID,
        ResponseMessage::OperationState errorState);

    void sendErrorMessageOnFinalAmountsConfiguration();

    const string logHeader() const;

protected:
    Contractor::Shared mCoordinator;

    TrustLineAmount mTransactionAmount;

    // this field indicates that transaction should be rejected on voting stage
    bool mTransactionShouldBeRejected;
};


#endif //GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H
