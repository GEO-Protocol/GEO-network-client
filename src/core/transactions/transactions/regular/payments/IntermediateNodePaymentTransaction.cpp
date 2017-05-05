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
{
    mStep = Stages::IntermediateNode_PreviousNeighborRequestProcessing;
}

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
    case Stages::IntermediateNode_PreviousNeighborRequestProcessing:
        return runPreviousNeighborRequestProcessingStage();

    case Stages::IntermediateNode_CoordinatorRequestProcessing:
        return runCoordinatorRequestProcessingStage();

    case Stages::IntermediateNode_NextNeighborResponseProcessing:
        return runNextNeighborResponseProcessingStage();

    case Stages::Common_FinalPathConfigurationChecking:
        return runFinalPathConfigurationProcessingStage();

    case Stages::IntermediateNode_ReservationProlongation:
        return runReservationProlongationStage();

    case Stages::Common_FinalPathsConfigurationChecking:
        return runFinalPathsConfigurationProcessingStage();

    case Stages::Common_VotesChecking:
        return runVotesCheckingStage();

    default:
        throw RuntimeError(
            "IntermediateNodePaymentTransaction::run: "
            "unexpected stage occurred.");
    }
}

pair<BytesShared, size_t> IntermediateNodePaymentTransaction::serializeToBytes()
{
    throw Exception("Not implemented");
}


TransactionResult::SharedConst IntermediateNodePaymentTransaction::runPreviousNeighborRequestProcessingStage()
{
    info() << "runPreviousNeighborRequestProcessingStage";
    const auto kNeighbor = mMessage->senderUUID;
    info() << "Init. intermediate payment operation from node (" << kNeighbor << ")";
    info() << "Requested amount reservation: " << mMessage->amount();


    // Note: (copy of shared pointer is required)
    const auto kIncomingAmount = mTrustLines->availableIncomingAmount(kNeighbor);
    const auto kReservationAmount =
        min(mMessage->amount(), *kIncomingAmount);

    if (0 == kReservationAmount || ! reserveIncomingAmount(kNeighbor, kReservationAmount, mMessage->pathUUID())) {
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            mMessage->pathUUID(),
            ResponseMessage::Rejected);
    } else {
        mLastReservedAmount = kReservationAmount;
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            mMessage->pathUUID(),
            ResponseMessage::Accepted,
            kReservationAmount);
    }

    mStep = Stages::IntermediateNode_CoordinatorRequestProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationRequest},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runCoordinatorRequestProcessingStage()
{
    info() << "runCoordinatorRequestProcessingStage";
    if (! contextIsValid(Message::Payments_CoordinatorReservationRequest))
        return reject("No coordinator request received. Rolled back.");


    info() << "Coordinator further reservation request received.";


    // TODO: add check for previous nodes amount reservation


    const auto kMessage = popNextMessage<CoordinatorReservationRequestMessage>();
    const auto kNextNode = kMessage->nextNodeInPathUUID();
    mCoordinator = kMessage->senderUUID;
    mLastProcessedPath = kMessage->pathUUID();



    // Note: copy of shared pointer is required
    const auto kOutgoingAmount = mTrustLines->availableOutgoingAmount(kNextNode);
    TrustLineAmount reservationAmount = min(
        kMessage->amount(),
        *kOutgoingAmount);

    if (0 == reservationAmount || ! reserveOutgoingAmount(kNextNode, reservationAmount, kMessage->pathUUID())) {
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            kMessage->pathUUID(),
            ResponseMessage::Rejected);

        info() << "No amount reservation is possible. Rolled back.";
        rollBack(kMessage->pathUUID());
        mStep = Stages::IntermediateNode_ReservationProlongation;
        // TODO correct delay time
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay(kMaxPathLength));
    }


    mLastReservedAmount = reservationAmount;
    sendMessage<IntermediateNodeReservationRequestMessage>(
        kNextNode,
        currentNodeUUID(),
        currentTransactionUUID(),
        kMessage->pathUUID(),
        reservationAmount);

    clearContext();
    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runNextNeighborResponseProcessingStage()
{
    info() << "runNextNeighborResponseProcessingStage";
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        info() << "No valid amount reservation response received. Rolled back.";
        rollBack(mLastProcessedPath);
        mStep = Stages::IntermediateNode_ReservationProlongation;
        // TODO correct delay time
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay(kMaxPathLength));
    }


    const auto kMessage = popNextMessage<IntermediateNodeReservationResponseMessage>();
    const auto kContractor = kMessage->senderUUID;


    if (kMessage->state() == IntermediateNodeReservationResponseMessage::Rejected){
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            kMessage->pathUUID(),
            ResponseMessage::Rejected);
        info() << "Amount reservation rejected by the neighbor node.";
        rollBack(kMessage->pathUUID());
        mStep = Stages::IntermediateNode_ReservationProlongation;
        // TODO correct delay time
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay(kMaxPathLength));
    }


    info() << "(" << kContractor << ") accepted amount reservation.";

    // shortage local reservation on current path
    for (const auto &nodeReservation : mReservations) {
        for (const auto &pathUUIDAndreservation : nodeReservation.second) {
            if (pathUUIDAndreservation.first == kMessage->pathUUID()) {
                shortageReservation(
                    nodeReservation.first,
                    pathUUIDAndreservation.second,
                    mLastReservedAmount,
                    pathUUIDAndreservation.first);
            }
        }
    }

    sendMessage<CoordinatorReservationResponseMessage>(
        mCoordinator,
        currentNodeUUID(),
        currentTransactionUUID(),
        kMessage->pathUUID(),
        ResponseMessage::Accepted,
        mLastReservedAmount);

    mStep = Stages::Common_FinalPathConfigurationChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalPathConfiguration},
        maxNetworkDelay(kMaxPathLength));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runReservationProlongationStage()
{
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        mMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();
        return runPreviousNeighborRequestProcessingStage();
    }
    info() << "runReservationProlongationStage";
    // In case if participants votes message is already received -
    // there is no need to prolong reservation, transaction may be proceeded.
    if (! mContext.empty())
        if (mContext.at(0)->typeID() == Message::Payments_ParticipantsVotes) {
            mStep = Stages::Common_VotesChecking;
            return runVotesCheckingStage();
        }

    return reject("No participants votes message received. Rolling back.");
}

void IntermediateNodePaymentTransaction::deserializeFromBytes(
    BytesShared buffer)
{
    throw Exception("Not implemented");
}

const string IntermediateNodePaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[IntermediateNodePaymentTA: " << currentTransactionUUID() << "] ";

    return s.str();
}
