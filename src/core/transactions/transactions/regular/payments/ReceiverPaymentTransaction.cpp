#include "ReceiverPaymentTransaction.h"

ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    ReceiverInitPaymentMessage::Shared message,
    TrustLinesManager *trustLines,
    Logger *log) :

    BasePaymentTransaction(
        BaseTransaction::ReceiverPaymentTransaction,
        message->transactionUUID()),
    mMessage(message),
    mTrustLines(trustLines),
    mLog(log)
{}

ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log) :

    // TODO: use deserialization constructor
    BasePaymentTransaction(
        BaseTransaction::ReceiverPaymentTransaction),
    mTrustLines(trustLines),
    mLog(log){

    deserializeFromBytes(buffer);
}

TransactionResult::SharedConst ReceiverPaymentTransaction::run() {

    switch (mStep) {
    case 1:
        return initOperation();

    case 2:
        return processAmountReservationStage();

    default:
        throw ValueError(
            "ReceiverPaymentTransaction::run(): "
            "invalid stage number occurred");
    }

    // TODO: remove this
    return make_shared<TransactionResult>(
        TransactionState::exit());
}

pair<BytesShared, size_t> ReceiverPaymentTransaction::serializeToBytes() {
    throw ValueError("Not implemented");
}

void ReceiverPaymentTransaction::deserializeFromBytes(BytesShared buffer) {
    throw ValueError("Not implemented");
}

const string ReceiverPaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiverPaymentTA: "
      << UUID().stringUUID()
      << "] ";

    return s.str();
}

TransactionResult::Shared ReceiverPaymentTransaction::initOperation() {

#ifdef TRANSACTIONS_LOG
    {
        auto info = mLog->info(logHeader());
        info << "Init. payment op. from (" << mMessage->senderUUID() << ")";
    }
    {
        auto info = mLog->info(logHeader());
        info << "Operation amount: " << mMessage->amount();
    }
#endif
    // TODO: (optimisation)
    // Check if total incoming possibilities of the node are <= of the payment amount.
    // If not - there is no reason to process the operation at all.
    // (reject operation)


    // Inform the coordinator about initialised operation
    auto message = make_shared<ReceiverApprovePaymentMessage>(
        nodeUUID(),
        UUID(),
        ReceiverApprovePaymentMessage::Accepted);
    addMessage(message, mMessage->senderUUID());

    increaseStepsCounter();

    const auto maxWaitTimeout = kMaxNodesCount * kMaxMessageTransferLagMSec;
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypes({}, maxWaitTimeout));
}

TransactionResult::Shared ReceiverPaymentTransaction::processAmountReservationStage() {

    if (mContext.empty()) {
#ifdef TRANSACTIONS_LOG
        {
            auto info = mLog->info(logHeader());
            info << "No amount reservation request received. "
                    "Transaction may not be proceed. Stoping";
        }

        return make_shared<TransactionResult>(
            TransactionState::exit());
#endif
    }

    return make_shared<TransactionResult>(
        TransactionState::exit());
}
