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
        const NodeUUID &currentNodeUUID,
        ReceiverInitPaymentRequestMessage::ConstShared message,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &log,
        SubsystemsController *subsystemsController);

    ReceiverPaymentTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &log,
        SubsystemsController *subsystemsController);

    TransactionResult::SharedConst run()
        noexcept;

protected:
    TransactionResult::SharedConst runInitialisationStage();
    TransactionResult::SharedConst runAmountReservationStage();
    TransactionResult::SharedConst runClarificationOfTransactionBeforeVoting();
    TransactionResult::SharedConst runVotesCheckingStageWithCoordinatorClarification();
    TransactionResult::SharedConst runClarificationOfTransactionDuringVoting();

protected:
    // Receiver must must save payment operation into history.
    // Therefore this methods are overriden.
    TransactionResult::SharedConst approve();

protected:
    void savePaymentOperationIntoHistory();

    bool checkReservationsDirections() const;

    void runBuildThreeNodesCyclesSignal();

    const string logHeader() const;

protected:
    const ReceiverInitPaymentRequestMessage::ConstShared mMessage;

    // this field indicates that transaction should be rejected on voting stage
    // it used only for Receiver
    bool mTransactionShouldBeRejected;
};


#endif //GEO_NETWORK_CLIENT_RECEIVERPAYMENTTRANSACTION_H
