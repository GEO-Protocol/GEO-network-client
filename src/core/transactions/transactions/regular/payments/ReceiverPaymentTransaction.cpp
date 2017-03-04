#include "ReceiverPaymentTransaction.h"


ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    const NodeUUID &currentNodeUUID,
    ReceiverInitPaymentMessage::ConstShared message,
    TrustLinesManager *trustLines,
    Logger *log) :

    BasePaymentTransaction(
        BaseTransaction::ReceiverPaymentTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        trustLines,
        log),
    mMessage(message)
{}

ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log) :

    BasePaymentTransaction(
        BaseTransaction::ReceiverPaymentTransaction,
        buffer,
        trustLines,
        log)
{}

/**
 * @throws RuntimeError in case if current stage is invalid.
 * @throws Exception from inner logic
 */
TransactionResult::SharedConst ReceiverPaymentTransaction::run() {

    switch (mStep) {
    case 1:
        return initOperation();

    case 2:
        return processAmountReservationStage();

    default:
        throw RuntimeError(
            "ReceiverPaymentTransaction::run(): "
            "invalid stage number occurred");
    }
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
    s << "[ReceiverPaymentTA: " << UUID().stringUUID() << "] ";
    return s.str();
}

TransactionResult::Shared ReceiverPaymentTransaction::initOperation() {

    info() << "Init. payment op. from (" << mMessage->senderUUID() << ")";
    info() << "Operation amount: " << mMessage->amount();

    // TODO: (optimisation)
    // Check if total incoming possibilities of the node are <= of the payment amount.
    // If not - there is no reason to process the operation at all.
    // (reject operation)


    // Inform the coordinator about initialised operation
    sendMessage(
        make_shared<ReceiverApprovePaymentMessage>(
            nodeUUID(),
            UUID(),
            ReceiverApprovePaymentMessage::Accepted),
        mMessage->senderUUID());


    increaseStepsCounter();
    const auto maxWaitTimeout = kMaxNodesCount * kMaxMessageTransferLagMSec;
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypes({}, maxWaitTimeout));
}

TransactionResult::Shared ReceiverPaymentTransaction::processAmountReservationStage()
{
    if (mContext.empty()) {
        info() << "No amount reservation request received. "
                  "Transaction may not be proceed. Stoping";

        // By the protocol, receiver node must not retunr anything or log something.
        // So, it simply stops the transaction.
        return make_shared<TransactionResult>(
            TransactionState::exit());
    }


//    throw NotImplementedError("dfdf");


    return make_shared<TransactionResult>(
        TransactionState::exit());
}
