#include "ReceiverPaymentTransaction.h"

ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    ReceiverInitPaymentMessage::Shared message,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        BaseTransaction::ReceiverPaymentTransaction),
    mMessage(message),
    mTrustLines(trustLines),
    mLog(log){
}

ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        BaseTransaction::ReceiverPaymentTransaction),
    mTrustLines(trustLines),
    mLog(log){

    deserializeFromBytes(buffer);
}

const TransactionUUID &ReceiverPaymentTransaction::UUID() const {

    return mTransactionUUID;
}

TransactionResult::SharedConst ReceiverPaymentTransaction::run() {

    switch (mStep) {
        case 1: {
            return initOperation();



        }

        case 2: {

        }


    }

    return TransactionResult::SharedConst(
        new TransactionResult(
            TransactionState::exit()
        )
    );
}

pair<BytesShared, size_t> ReceiverPaymentTransaction::serializeToBytes() {
    throw ValueError("Not implemented");
}

void ReceiverPaymentTransaction::deserializeFromBytes(BytesShared buffer) {
    throw ValueError("Not implemented");
}

TransactionResult::Shared ReceiverPaymentTransaction::initOperation() {

#ifdef TRANSACTIONS_LOG
    auto info = mLog->info("ReceiverPaymentTransaction");
#endif

    // todo: optimisation
    // Check if total incoming possibilities of the node are <= of the payment amount.
    // If not - there is no reason to process the operation at all.
    // (reject operation)

    // todo: init processing structure

    // Inform the coordinator about initialised operation
    auto message =
        make_shared<OperationStateMessage>(
            OperationStateMessage::Accepted);

    addMessage(message, mMessage->senderUUID());

#ifdef TRANSACTIONS_LOG
    info
         << "Payment operation from node "
         << mMessage->senderUUID()
         << "succesfully initialised";
#endif

    // Begin receiving amount locks,
    // but no longer than 3s for first payment operation.
    mStep += 1;
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypes({}, 3*1000));
}

TransactionResult::Shared ReceiverPaymentTransaction::processAmountBlockingStage() {

#ifdef TRANSACTIONS_LOG
    auto info = mLog->info("ReceiverPaymentTransaction");
#endif

    // todo: check for amount blocking messages

#ifdef TRANSACTIONS_LOG
    info
         << "No amount block message was received. "
         << "Transaction is now would be closed.";
#endif

    return make_shared<TransactionResult>(
        TransactionState::exit());
}