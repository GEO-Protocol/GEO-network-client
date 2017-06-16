#include "CycleCloserIntermediateNodeTransaction.h"

CycleCloserIntermediateNodeTransaction::CycleCloserIntermediateNodeTransaction(
    const NodeUUID& currentNodeUUID,
    IntermediateNodeCycleReservationRequestMessage::ConstShared message,
    TrustLinesManager* trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log) :

    BasePaymentTransaction(
        BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log),
    mMessage(message)
{
    mStep = Stages::IntermediateNode_PreviousNeighborRequestProcessing;
}

CycleCloserIntermediateNodeTransaction::CycleCloserIntermediateNodeTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager* trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log) :
    BasePaymentTransaction(
        buffer,
        nodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log)
{}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::run()
    noexcept {
    try {
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

            case Stages::Common_VotesChecking:
                return runVotesCheckingStage();

            case Stages::Common_Recovery:
                return runVotesRecoveryParentStage();
            default:
                throw RuntimeError(
                    "IntermediateNodePaymentTransaction::run: "
                        "unexpected stage occurred.");
        }
    } catch (...) {
        recover("Something happens wrong in method run(). Transaction will be recovered");
    }
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runPreviousNeighborRequestProcessingStage()
{
    debug() << "runPreviousNeighborRequestProcessingStage";
    const auto kNeighbor = mMessage->senderUUID;
    debug() << "Init. intermediate payment operation from node (" << kNeighbor << ")";
    debug() << "Requested amount reservation: " << mMessage->amount();

    mCycleLength = mMessage->cycleLength();
    TrustLineAmount kReservationAmount = TrustLine::kZeroAmount();
    // Note: (copy of shared pointer is required)
    try {
        const auto kIncomingAmount = mTrustLines->availableIncomingCycleAmount(kNeighbor);
        kReservationAmount = min(mMessage->amount(), *kIncomingAmount);
    } catch (NotFoundError &e) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);
        error() << "Path is not valid: previous node is not neighbor of current one. Rejected.";
        return resultDone();
    }

    if (0 == kReservationAmount || ! reserveIncomingAmount(kNeighbor, kReservationAmount, 0)) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);
        return resultDone();
    }

    mLastReservedAmount = kReservationAmount;
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        kNeighbor,
        currentNodeUUID(),
        currentTransactionUUID(),
        ResponseCycleMessage::Accepted,
        kReservationAmount);

    mStep = Stages::IntermediateNode_CoordinatorRequestProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationRequest},
        maxNetworkDelay(3));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runCoordinatorRequestProcessingStage()
{
    debug() << "runCoordinatorRequestProcessingStage";
    if (! contextIsValid(Message::Payments_CoordinatorCycleReservationRequest))
        return reject("No coordinator request received. Rolled back.");

    debug() << "Coordinator further reservation request received.";


    // TODO: add check for previous nodes amount reservation


    const auto kMessage = popNextMessage<CoordinatorCycleReservationRequestMessage>();
    const auto kNextNode = kMessage->nextNodeInPathUUID();
    mCoordinator = kMessage->senderUUID;
    mCycleLength = kMessage->cycleLength();

    TrustLineAmount reservationAmount = TrustLine::kZeroAmount();
    try {
        // Note: copy of shared pointer is required
        const auto kOutgoingAmount = mTrustLines->availableOutgoingCycleAmount(kNextNode);
        reservationAmount = min(kMessage->amount(), *kOutgoingAmount);
    } catch (NotFoundError &e) {
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);

        error() << "Path is not valid: next node is not neighbor of current one. Rejected.";
        rollBack();
        return resultDone();
    }

    if (0 == reservationAmount || ! reserveOutgoingAmount(kNextNode, reservationAmount, 0)) {
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);

        debug() << "No amount reservation is possible. Rolled back.";
        rollBack();
        return resultDone();
    }

    mLastReservedAmount = reservationAmount;
    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        kNextNode,
        currentNodeUUID(),
        currentTransactionUUID(),
        reservationAmount,
        mCycleLength);

    clearContext();
    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runNextNeighborResponseProcessingStage()
{
    debug() << "runNextNeighborResponseProcessingStage";
    if (! contextIsValid(Message::Payments_IntermediateNodeCycleReservationResponse)) {
        debug() << "No valid amount reservation response received. Rolled back.";
        rollBack();
        return resultDone();
    }

    const auto kMessage = popNextMessage<IntermediateNodeCycleReservationResponseMessage>();
    const auto kContractor = kMessage->senderUUID;

    if (kMessage->state() == IntermediateNodeCycleReservationResponseMessage::Rejected){
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);
        debug() << "Amount reservation rejected by the neighbor node.";
        rollBack();
        return resultDone();
    }

    debug() << "(" << kContractor << ") accepted amount reservation.";

    // shortage local reservation on current path
    for (const auto &nodeReservation : mReservations) {
        for (const auto &pathUUIDAndreservation : nodeReservation.second) {
            shortageReservation(
                nodeReservation.first,
                pathUUIDAndreservation.second,
                mLastReservedAmount,
                pathUUIDAndreservation.first);
        }
    }

    sendMessage<CoordinatorCycleReservationResponseMessage>(
        mCoordinator,
        currentNodeUUID(),
        currentTransactionUUID(),
        ResponseCycleMessage::Accepted,
        mLastReservedAmount);

    mStep = Stages::Common_FinalPathConfigurationChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalPathCycleConfiguration},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runReservationProlongationStage()
{
    debug() << "runReservationProlongationStage";
    // In case if participants votes message is already received -
    // there is no need to prolong reservation, transaction may be proceeded.
    // Node is clarifying of coordinator if transaction is still alive
    if (!contextIsValid(Message::Payments_ParticipantsVotes)) {
        return reject("No participants votes message received. Transaction was closed. Rolling Back");
    }
    mStep = Stages::Common_VotesChecking;
    return runVotesCheckingStage();
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runFinalPathConfigurationProcessingStage()
{
    debug() << "runFinalPathConfigurationProcessingStage";
    if (! contextIsValid(Message::Payments_FinalPathCycleConfiguration))
        return reject("No final paths configuration was received from the coordinator. Rejected.");


    const auto kMessage = popNextMessage<FinalPathCycleConfigurationMessage>();

    // TODO: check if message was really received from the coordinator;


    debug() << "Final payment path configuration received";

    // path was cancelled, drop all reservations belong it
    if (kMessage->amount() == 0) {
        rollBack();
        return resultDone();
    } else {

        // Shortening all reservations that belongs to this node and path.
        for (const auto &nodeAndReservations : mReservations) {
            for (const auto &pathUUIDAndReservation : nodeAndReservations.second) {
                shortageReservation(
                    nodeAndReservations.first,
                    pathUUIDAndReservation.second,
                    kMessage->amount(),
                    pathUUIDAndReservation.first);
            }
        }
    }

    mStep = Stages::IntermediateNode_ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxNetworkDelay(kMaxPathLength - 2));
}

void CycleCloserIntermediateNodeTransaction::deserializeFromBytes(
    BytesShared buffer)
{
    throw Exception("Not implemented");
}

pair<BytesShared, size_t> CycleCloserIntermediateNodeTransaction::serializeToBytes()
{
    throw Exception("Not implemented");
}

const string CycleCloserIntermediateNodeTransaction::logHeader() const
{
    stringstream s;
    s << "[CycleCloserIntermediateNodeTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
