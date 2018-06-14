#ifndef GEO_NETWORK_CLIENT_CYCLECLOSERINTERMEDIATENODETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLECLOSERINTERMEDIATENODETRANSACTION_H

#include "base/BasePaymentTransaction.h"
#include "../../../../cycles/CyclesManager.h"

class CycleCloserIntermediateNodeTransaction : public BasePaymentTransaction {

public:
    typedef shared_ptr<CycleCloserIntermediateNodeTransaction> Shared;
    typedef shared_ptr<const CycleCloserIntermediateNodeTransaction> ConstShared;

public:
    CycleCloserIntermediateNodeTransaction(
        const NodeUUID &currentNodeUUID,
        IntermediateNodeCycleReservationRequestMessage::ConstShared message,
        TrustLinesManager *trustLines,
        CyclesManager *cyclesManager,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        Keystore *keystore,
        Logger &log,
        SubsystemsController *subsystemsController);

    CycleCloserIntermediateNodeTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager* trustLines,
        CyclesManager *cyclesManager,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        Keystore *keystore,
        Logger &log,
        SubsystemsController *subsystemsController);

    TransactionResult::SharedConst run()
    noexcept;

    /**
     * @return coordinator UUID of current transaction
     */
    const NodeUUID& coordinatorUUID() const;

    /**
     * @return length of cycle which is closing by current transaction
     * used in CyclesManager for resolving cycle closing conflicts
     */
    const SerializedPathLengthSize cycleLength() const;

protected:
    /**
     * reaction on reservation request message from previous node on closing cycle
     * try reserve requested incoming amount and send reservation response
     */
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStage();

    /**
     * reaction on coordinator further reservation request message
     * try reserve requested outgoing amount to next node on closing cycle
     * and send reservation request message to this node
     */
    TransactionResult::SharedConst runCoordinatorRequestProcessingStage();

    /**
     * reaction on next node reservation response message
     * send reservation response to coordinator
     */
    TransactionResult::SharedConst runNextNeighborResponseProcessingStage();

    /**
     * reaction on message with final amount configuration from coordinator
     * update all reservations according to received final configuration
     */
    TransactionResult::SharedConst runFinalPathConfigurationProcessingStage();

    TransactionResult::SharedConst runFinalPathConfigurationCoordinatorConfirmation();

    TransactionResult::SharedConst runFinalReservationsNeighborConfirmation();

    /**
     * continue reaction on coordinator further reservation request message
     * after waiting for closing conflicted transaction and releasing it amount
     */
    TransactionResult::SharedConst runCoordinatorRequestProcessingStageAgain();

    /**
     * continue reaction on reservation request message from previous node
     * after waiting for closing conflicted transaction and releasing it amount
     */
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStageAgain();

    /**
     * reaction on receiving participants votes message firstly,
     * add own vote to message and send it to next participant,
     * if no participants votes message received then send message (TTL)
     * to coordinator with request if transaction is still alive
     */
    TransactionResult::SharedConst runVotesCheckingStageWithPossibleTTL();

protected:
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
     * (there are should be one incoming and one outgoing reservations with the same amount)
     * @return true if reservations are valid
     */
    bool checkReservationsDirections() const;

    const string logHeader() const;

protected:
    // message on which current transaction was started
    IntermediateNodeCycleReservationRequestMessage::ConstShared mMessage;

    TrustLineAmount mLastReservedAmount;
    NodeUUID mCoordinator;
    SerializedPathLengthSize mCycleLength;

    // fields, wor continue process coordinator request after releasing conflicted reservation
    // transaction on which reservation we pretend
    TransactionUUID mConflictedTransaction;
    NodeUUID mNextNode;
    NodeUUID mPreviousNode;
    TrustLineAmount mReservationAmount;

    // for resolving reservation conflicts
    CyclesManager *mCyclesManager;

private:
    // time waiting (in milliseconds) for closing of conflicted transaction
    const uint16_t kWaitingForReleasingAmountMSec = 50;
};


#endif //GEO_NETWORK_CLIENT_CYCLECLOSERINTERMEDIATENODETRANSACTION_H
