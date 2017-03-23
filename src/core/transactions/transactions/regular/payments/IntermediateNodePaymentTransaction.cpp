#include "IntermediateNodePaymentTransaction.h"


IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    const NodeUUID& currentNodeUUID,
    IntermediateNodeReservationRequestMessage::ConstShared message,
    TrustLinesManager* trustLines,
    Logger* log) :

    BasePaymentTransaction(
        BaseTransaction::IntermediateNodePaymentTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        trustLines,
        log),
    mMessage(message)
{}

IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    BytesShared buffer,
    TrustLinesManager* trustLines,
    Logger* log) :

    BasePaymentTransaction(
        BaseTransaction::IntermediateNodePaymentTransaction,
        buffer,
        trustLines,
        log)
{}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::run()
{
    switch (mStep) {
    case Stages::PreviousNeighborRequestProcessing:
        return runPreviousNeighborRequestProcessingStage();

    case Stages::CoordinatorRequestProcessing:
        return runCoordinatorRequestProcessingStage();

    case Stages::NextNeighborResponseProcessing:
        return runNextNeighborResponseProcessingStage();

    case Stages::ReservationProlongation:
        return runReservationProlongationStage();

    default:
        throw RuntimeError(
            "IntermediateNodePaymentTransaction::run: unexpected stage occured.");
    }
}

pair<BytesShared, size_t> IntermediateNodePaymentTransaction::serializeToBytes()
{
    throw Exception("Not implemented");
}


TransactionResult::SharedConst IntermediateNodePaymentTransaction::runPreviousNeighborRequestProcessingStage()
{
    // This method launches from message received in transaction constructor.
    // No context validation is needed here.

    const auto kNeighbor = mMessage->senderUUID();


    info() << "Init. intermediate payment operation from node - (" << kNeighbor << ")";
    info() << "Amount reservation request: " << mMessage->amount();

    // note: (copy of shared pointer is required)
    const auto incomingAmount = mTrustLines->availableIncomingAmount(kNeighbor);
    TrustLineAmount reservationAmount =
        min(mMessage->amount(), *incomingAmount);

    debug() << reservationAmount;

    if (0 == reservationAmount || ! reserveIncomingAmount(kNeighbor, reservationAmount)) {
        info() << "No incoming amount reservation is posible. Canceling.";

        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            nodeUUID(),
            UUID(),
            ResponseMessage::Rejected);
        return exit();

    } else {
        mLastReservedAmount = reservationAmount;
        info() << "Locally reserved amount: " << reservationAmount;

        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            nodeUUID(),
            UUID(),
            ResponseMessage::Accepted);
        info() << "Neighbor reservation accepted.";

        mStep = Stages::CoordinatorRequestProcessing;

        const auto kResponseTripTime = kMaxMessageTransferLagMSec * 2;
        return resultWaitForMessageTypes(
            {Message::Payments_CoordinatorReservationRequest},
            kResponseTripTime);
    }
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runCoordinatorRequestProcessingStage()
{
    info() << "Processing coordinator request";

    if (! contextIsValid(Message::Payments_CoordinatorReservationRequest)) {
        return exit();
    }

    // TODO: add check for previous nodes amount reservation


    const auto kMessage = popNextMessage<CoordinatorReservationRequestMessage>();
    const auto kNextNode = kMessage->nextNodeInPathUUID();
    mCoordinator = kMessage->senderUUID();


    // TODO: check previous node reservation


    //
    // Local reservation

    // note: don't delete this (copy of shared pointer is required)
    const auto kOutgoingAmount = mTrustLines->availableOutgoingAmount(kNextNode);
    TrustLineAmount reservationAmount = min(
        kMessage->amount(),
        *kOutgoingAmount);

    if (0 == reservationAmount || ! reserveAmount(kNextNode, reservationAmount)) {
        info() << "No amount reservation is posible. Canceling.";
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            nodeUUID(),
            UUID(),
            ResponseMessage::Rejected,
            0);
        return exit();
    }

    mLastReservedAmount = reservationAmount;
    info() << "Locally reserved amount: " << reservationAmount;
    info() << "Waiting for neighbor node amount reservation approve.";


    //
    // Process neighbor node reservation
    sendMessage<IntermediateNodeReservationRequestMessage>(
        kNextNode,
        nodeUUID(),
        UUID(),
        reservationAmount);

    clearContext();

    mStep = Stages::NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse},
        kMaxMessageTransferLagMSec);
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runNextNeighborResponseProcessingStage()
{
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        return exit();
    }

    const auto kMessage = popNextMessage<IntermediateNodeReservationResponseMessage>();
    const auto kContractor = kMessage->senderUUID();


    if (kMessage->state() == IntermediateNodeReservationResponseMessage::Rejected){
        // TODO: check trust line consistency

        error() << "(" << kContractor << ") rejected amount reservation. Canceling";
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            nodeUUID(),
            UUID(),
            ResponseMessage::Rejected);
        return exit();
    }

    info() << "(" << kContractor << ") accepted amount reservation";
    sendMessage<CoordinatorReservationResponseMessage>(
        mCoordinator,
        nodeUUID(),
        UUID(),
        ResponseMessage::Accepted,
        mLastReservedAmount);


    mStep = Stages::ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        kCoordinatorPingTimeoutMSec);
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runReservationProlongationStage()
{
    // todo: add amount reservation

    // In case if participants votes message is already received -
    // there is no need to prolong reservation, transaction may be proceeded.
    if (! mContext.empty())
        if (mContext.at(0)->typeID() == Message::Payments_ParticipantsVotes) {
            mStep = Stages::VotesProcessing;
            return runVotesProcessingStage();
        }



    info() << "Reservation prolongation stage";
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        kCoordinatorPingTimeoutMSec);
}


//TransactionResult::SharedConst IntermediateNodePaymentTransaction::processReservationAssumingCoordiinatorIsNeighbour()
//{
//    const auto kCoordinator = mMessage->senderUUID();


//    info() << "Processing reservation request assuming coordinator is neighbor.";

//    // note: don't delete this (copy of shared pointer is required)
//    const auto incomingAmount = availableIncomingAmount(kCoordinator);
//    TrustLineAmount reservationAmount =
//        min(mMessage->amount(), *incomingAmount);

//    if (0 == reservationAmount) {
//        info() << "No amount reservation from coordinator is posible. Canceling.";

//        return rejectRequest();
//    }

//    if (! reserveIncomingAmount(kCoordinator, reservationAmount)) {
//        error() << "Can't reserve incoming amount " << reservationAmount;
//        error() << "Transaction may not be proceed. Stopping.";

//        return rejectRequest();
//    }

//    info() << "Successfully reserved " << reservationAmount << " for the coordinator.";

//    return processReservationAssumingCoordinatorIsRemoteNode();
//}

//TransactionResult::SharedConst IntermediateNodePaymentTransaction::processReservationAssumingCoordinatorIsRemoteNode()
//{
//    const auto kNextNode = mMessage->nextNodeInThePathUUID();


//    info() << "Processing reservation request to the (" << kNextNode << ").";

//    // note: don't delete this (copy of shared pointer is required)
//    const auto outgoingAmount = availableAmount(kNextNode);
//    TrustLineAmount reservationAmount =
//        min(mMessage->amount(), *outgoingAmount);

//    if (0 == reservationAmount) {
//        info() << "No amount reservation to next node in path is posible. Canceling.";

//        return rejectRequest();
//    }

//    if (! reserveAmount(kNextNode, reservationAmount)) {
//        error() << "Can't reserve incoming amount " << reservationAmount;
//        error() << "Transaction may not be proceed. Stopping.";

//        return rejectRequest();
//    }

//    info() << "Reserved locally " << reservationAmount << " for the " << kNextNode;
//    return propagateReservation(reservationAmount);
//}

//TransactionResult::SharedConst IntermediateNodePaymentTransaction::rejectRequest()
//{
//    sendMessage(
//        make_shared<IntermediateNodeReservationResponseMessage>(
//            nodeUUID(),
//            UUID(),
//            IntermediateNodeReservationResponseMessage::Rejected),
//        mMessage->senderUUID());

//    return exit();
//}

//TransactionResult::SharedConst IntermediateNodePaymentTransaction::acceptRequest()
//{
//    sendMessage(
//        make_shared<IntermediateNodeReservationResponseMessage>(
//            nodeUUID(),
//            UUID(),
//            IntermediateNodeReservationResponseMessage::Accepted),
//        mMessage->senderUUID());

//    // TODO: Wait for shortages nj  message
//    return exit();
//}


//TransactionResult::SharedConst IntermediateNodePaymentTransaction::propagateReservation(
//    const TrustLineAmount &amount)
//{
//    info() << "Propagating reservation request to " << amount;


//}

//TransactionResult::SharedConst IntermediateNodePaymentTransaction::coordinatorRequestRejected(
//    const NodeUUID &coordinator)
//{
//    error() << "(" << kContractor << ") rejected amount reservation. Canceling";

//    sendMessage<CoordinatorReservationResponseMessage>(
//        coordinator,
//        nodeUUID(),
//        UUID(),
//        IntermediateNodeReservationResponseMessage::Rejected);

//    info() << "Reject reservation response sent";
//    return exit();
//}


void IntermediateNodePaymentTransaction::deserializeFromBytes(
    BytesShared buffer)
{
    throw Exception("Not implemented");
}

const string IntermediateNodePaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[IntermediateNodePaymentTA: " << UUID().stringUUID() << "] ";

    return s.str();
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runVotesProcessingStage()
{
    // If no votes message is present - transaction must be stopped.
    // All amount reservations would be automatically dropped.
    if (!contextIsValid(Message::Payments_ParticipantsVotes)) {
        error() << "No participants votes received. Canceling.";
        return exit();
    }

    const auto kCurrentNodeUUID = nodeUUID();
    auto message = popNextMessage<ParticipantsApprovingMessage>();
    if (message->containsRejectVote()) {
        info() << "Some node rejected the transaction, so it must be rolled back.";
        return exit();
    }

    // ToDo: check if node may sign the message
    // (previous nodes processing, etc)

    // TODO: check behavioud when message wasn't approved

    try {
        message->approve(kCurrentNodeUUID);

    } catch (NotFoundError &) {
        // It seems that current node wasn't listed in votes list.
        // This behavoud is possible only in case when one node takes part in 2 parallel transactions
        // that have the common UUID (transactions UUID collision).
        // The probability of this case is very small, but is present.
        //
        // In this case - the message must be simply ignored.

        clearContext();
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes}, 3000);
    }

    try {
        // Try to get next participant from the message.
        // In case if this node is the last node in votes list -
        // then transaction is almost ready to be closed.
        // Success propagation step is needed.
        const auto kNextParticipant = message->nextParticipant(kCurrentNodeUUID);

        // Current node is not last in the votes list.
        // Message must be transferred to the next node in the list.
        sendMessage(
            kNextParticipant,
            message);

        auto const kExpectedNodeProcessingDelay = 1500; // msec
        auto kMaxTimeout =
            (message->totalParticipantsCount() * kMaxMessageTransferLagMSec)
            + (message->totalParticipantsCount() * kExpectedNodeProcessingDelay);

        // Transaction must wait for next message, that must be already signed.
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes}, kMaxTimeout);

    } catch (NotFoundError &) {
        // There are no nodes left in this votes list,
        // so the current one is the last one.
        //
        // Message is aproved, and this means that other nodes has been approved it too.
        // Sucess propagation must be done now.

        info() << "Success propagation step!";
    }
}
