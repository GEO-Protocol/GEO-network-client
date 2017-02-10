#include "CoordinatorPaymentTransaction.h"

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    NodeUUID &currentNodeUUID,
    CreditUsageCommand::Shared command,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        BaseTransaction::CoordinatorPaymentTransaction, currentNodeUUID),
    mCommand(command),
    mTrustLines(trustLines),
    mLog(log){
}

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        BaseTransaction::CoordinatorPaymentTransaction),
    mTrustLines(trustLines),
    mLog(log){

    throw ValueError("Not implemented");
}

TransactionResult::Shared CoordinatorPaymentTransaction::run() {

    switch (mStep) {
        case 1:
            return initOperation();

        case 2:
            return processReceiverResponse();

        default:
            throw ValueError(
                "CoordinatorPaymentTransaction::run(): "
                    "invalid transaction step.");
    }
}


TransactionResult::Shared CoordinatorPaymentTransaction::initOperation() {

#ifdef TRANSACTIONS_LOG
    auto info = mLog->info("CoordinatorPaymentTransaction");
    info << logHeader()
         << "Initialising payment operation to node " << mCommand->contractorUUID();
#endif

    // todo: optimisation
    // Check if total outgoing possibilities of this node
    // are not smaller, than total operation amount.
    // In case if so - there is no reason to begin the operation:
    // current node would not be able to pay such an amount.

    auto message =
        make_shared<ReceiverInitPaymentMessage>(
            mCommand->amount());

    addMessage(
        message,
        mCommand->contractorUUID());

    // todo: wait for incoming message from receiver

    mStep += 1;
    return make_shared<TransactionResult>(
        TransactionState::awakeAfterMilliseconds(3000));
}

TransactionResult::Shared CoordinatorPaymentTransaction::processReceiverResponse() {

    // todo: (hsc) retry if no response (but no more than 5 times)

#ifdef TRANSACTIONS_LOG
    auto info = mLog->info("CoordinatorPaymentTransaction");
#endif

    // todo: process response if received

#ifdef TRANSACTIONS_LOG
    info << logHeader()
         << "No receiver response received. "
         << "Transaction will now be closed.";
#endif

    return make_shared<TransactionResult>(
        TransactionState::exit());
}

pair<BytesShared, size_t> CoordinatorPaymentTransaction::serializeToBytes() {
    throw ValueError("Not implemented");
}

void CoordinatorPaymentTransaction::deserializeFromBytes(BytesShared buffer) {
    throw ValueError("Not implemented");
}
