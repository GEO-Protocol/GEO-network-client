#include "ReceiverPaymentTransaction.h"


ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    const NodeUUID &currentNodeUUID,
    ReceiverInitPaymentRequestMessage::ConstShared message,
    TrustLinesManager *trustLines,
    Logger *log) :

    BasePaymentTransaction(
        BaseTransaction::ReceiverPaymentTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        trustLines,
        log),
    mMessage(message)
{
    cout << "\n\n P:" << mTrustLines << "\n\n" << endl;
}

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
TransactionResult::SharedConst ReceiverPaymentTransaction::run()
{
    switch (mStep) {
    case Stages::CoordinatorRequestApprooving:
        return initOperation();

    case Stages::AmountReservationsProcessing:
        return processAmountReservationStage();

    default:
        throw RuntimeError(
            "ReceiverPaymentTransaction::run(): "
            "invalid stage number occurred");
    }
}

pair<BytesShared, size_t> ReceiverPaymentTransaction::serializeToBytes()
{
    throw ValueError("Not implemented");
}

void ReceiverPaymentTransaction::deserializeFromBytes(BytesShared buffer)
{
    throw ValueError("Not implemented");
}

const string ReceiverPaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiverPaymentTA: " << UUID().stringUUID() << "] ";
    return s.str();
}

TransactionResult::Shared ReceiverPaymentTransaction::initOperation()
{
    const auto kCoordinator = mMessage->senderUUID();


    info() << "Operation initalise by the (" << kCoordinator << ")";
    info() << "Total operation amount: " << mMessage->amount();


    // TODO: (optimisation)
    // Check if total incoming possibilities of the node are <= of the payment amount.
    // If not - there is no reason to process the operation at all.
    // (reject operation)


    sendMessage<ReceiverInitPaymentResponseMessage>(
        kCoordinator,
        nodeUUID(),
        UUID(),
        ReceiverInitPaymentResponseMessage::Accepted);


    mStep = Stages::AmountReservationsProcessing;

    const auto kEstimatedTransactionProcessingTimeoutOnTheNode = 1000; //msec
    const auto kEstimatedPathProcessingTimeout =
        (kMaxNodesCount * kMaxMessageTransferLagMSec) +
        (kMaxNodesCount * kEstimatedTransactionProcessingTimeoutOnTheNode);
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationRequest},
        kEstimatedPathProcessingTimeout);
}

TransactionResult::Shared ReceiverPaymentTransaction::processAmountReservationStage()
{
    if (! validateContext(Message::Payments_IntermediateNodeReservationRequest)) {
        return exit();
    }


    const auto kMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();
    const auto kNeighbor = kMessage->senderUUID();

    info() << "Amount reservation request received from " << kNeighbor;
    info() << "Requested amount: " << kMessage->amount();


    // note: don't delete this (copy of shared pointer is required)
    const auto kAvailableAmount = mTrustLines->availableIncomingAmount(kNeighbor);
    TrustLineAmount reservationAmount = min(
        kMessage->amount(),
        *kAvailableAmount);

    if (0 == reservationAmount || ! reserveIncomingAmount(kNeighbor, reservationAmount)) {
        info() << "No amount reservation is posible for this node.";
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            nodeUUID(),
            UUID(),
            ResponseMessage::Rejected);

        goto FURTHER_REQUESTS_PROCESSING;
    }

    info() << "Reserved locally: " << reservationAmount;
    sendMessage<IntermediateNodeReservationResponseMessage>(
        kNeighbor,
        nodeUUID(),
        UUID(),
        ResponseMessage::Accepted);


FURTHER_REQUESTS_PROCESSING:
    const auto kEstimatedTransactionProcessingTimeoutOnTheNode = 1000; //msec
    const auto kEstimatedPathProcessingTimeout =
        (kMaxNodesCount * kMaxMessageTransferLagMSec) +
        (kMaxNodesCount * kEstimatedTransactionProcessingTimeoutOnTheNode);

    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationRequest},
        kEstimatedPathProcessingTimeout);
}
