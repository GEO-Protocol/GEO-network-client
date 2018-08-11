#ifndef GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H

#include "base/BasePaymentTransaction.h"
#include "../../../../interface/visual_interface/interface/VisualInterface.h"
#include "../../../../interface/visual_interface/visual/VisualResult.h"

class ReceiverPaymentTransaction:
    public BasePaymentTransaction {

public:
    typedef shared_ptr<ReceiverPaymentTransaction> Shared;
    typedef shared_ptr<const ReceiverPaymentTransaction> ConstShared;

public:
    ReceiverPaymentTransaction(
        const NodeUUID &currentNodeUUID,
        ReceiverInitPaymentRequestMessage::ConstShared message,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        Keystore *keystore,
        Logger &log,
        SubsystemsController *subsystemsController,
        VisualInterface *visualInterface);

    ReceiverPaymentTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
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
     * @return UUID of coordinator node of current transaction
     */
    const NodeUUID& coordinatorUUID() const;

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

    const string logHeader() const;

protected:
    // message on which current transaction was started
    const ReceiverInitPaymentRequestMessage::ConstShared mMessage;

    // this field indicates that transaction should be rejected on voting stage
    bool mTransactionShouldBeRejected;

    VisualInterface *mVisualInterface;
};


#endif //GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H
