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
    mMessage(message),
    mTotalReserved(0)
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
{
    deserializeFromBytes(buffer);
}

/**
 * @throws RuntimeError in case if current stage is invalid.
 * @throws Exception from inner logic
 */
TransactionResult::SharedConst ReceiverPaymentTransaction::run()
{
    switch (mStep) {
    case Stages::CoordinatorRequestApprooving:
        return runInitialisationStage();

    case Stages::AmountReservationsProcessing:
        return runAmountReservationStage();

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

TransactionResult::Shared ReceiverPaymentTransaction::runInitialisationStage()
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

TransactionResult::Shared ReceiverPaymentTransaction::runAmountReservationStage()
{
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationRequest))
        return exit();


    const auto kMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();
    const auto kNeighbor = kMessage->senderUUID();

    if (! mTrustLines->isNeighbor(kNeighbor)) {
        // Message was sent from node, that is not listed in neighbors list.
        //
        // TODO: enchance this chek
        // Neighbor private public key must be used here.
        goto FURTHER_REQUESTS_PROCESSING;
    }


    info() << "Amount reservation request received from " << kNeighbor;
    info() << "Requested amount: " << kMessage->amount();

    // Note:
    // Don't delete (copy of shared pointer is required).
    const auto kAvailableAmount = mTrustLines->availableIncomingAmount(kNeighbor);
    if (kMessage->amount() > kAvailableAmount || ! reserveIncomingAmount(kNeighbor, kMessage->amount())) {
        // Receiver must not confirm reservation in case if requested amount is less than available.
        // Intermediate nodes may decrease requested reservation amount, but receiver must not do this.
        // It must stay syncronised with previous node.
        // So, in case if requested amount is less than available -
        // previous node must report about it to the coordinator.
        // In this case, receiver should even not receive reservation request at all.
        //
        // Also, this kind of problem may appear when two nodes are involved
        // in several COUNTER transactions.
        // In this case, balances may be reserved on the nodes,
        // but neighbor node may reject reservation,
        // because it already reserved amount or othe transactions,
        // that wasn't approved by the current node yet.
        //
        // In this case, reservation request must be rejected.

        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            nodeUUID(),
            UUID(),
            ResponseMessage::Rejected);

        goto FURTHER_REQUESTS_PROCESSING;

    } else {
        mTotalReserved += message->amount();
        info() << "Reserved locally: " << reservationAmount;

        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            nodeUUID(),
            UUID(),
            ResponseMessage::Accepted);

        // Checking if reserved amount is enought for the payment operation.
        // If so - move to the votes approving stage.

#ifdef DEBUG
        if (mTotalReserved > mMessage->amount()){
            error() << "Reserved amount is greater than requested.";
            assert(false);
        }
#endif

        if (mTotalReserved == mMessage->amount()) {
            // TODO: receiver must know, how many paths are involed into transaction.
            // This info helps to calculate max timeout,
            // that would be used for waiting for votes list message.

            const auto kMaxVaitingTimeout = 10000;

            mStep = Stages::VotesChecking;
            return resultWaitForMessageTypes(
                {Message::Payments_ParticipantsVotes},
                kMaxVaitingTimeout);
            }
    }

FURTHER_REQUESTS_PROCESSING:
    const auto kEstimatedTransactionProcessingTimeoutOnTheNode = 1000; //msec
    const auto kEstimatedPathProcessingTimeout =
        (kMaxNodesCount * kMaxMessageTransferLagMSec) +
        (kMaxNodesCount * kEstimatedTransactionProcessingTimeoutOnTheNode);

    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationRequest},
        kEstimatedPathProcessingTimeout);
}

TransactionResult::Shared ReceiverPaymentTransaction::runVotesCheckingStage()
{
    return exit();
}
