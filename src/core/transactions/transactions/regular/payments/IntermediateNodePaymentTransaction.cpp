#include "IntermediateNodePaymentTransaction.h"


IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    const NodeUUID& currentNodeUUID,
    IntermediateNodeReservationRequestMessage::ConstShared message,
    TrustLinesManager* trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger* log) :

    BasePaymentTransaction(
        BaseTransaction::IntermediateNodePaymentTransaction,
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

IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager* trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger* log) :
        BasePaymentTransaction(
            buffer,
            nodeUUID,
            trustLines,
            storageHandler,
            maxFlowCalculationCacheManager,
            log)
{}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::run()
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

            case Stages::Common_ClarificationTransaction:
                return runClarificationOfTransaction();

            case Stages::IntermediateNode_ReservationProlongation:
                return runReservationProlongationStage();

            case Stages::Common_VotesChecking:
                return runVotesCheckingStageWithCoordinatorClarification();

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

pair<BytesShared, size_t> IntermediateNodePaymentTransaction::serializeToBytes()
{
    throw Exception("Not implemented");
}


TransactionResult::SharedConst IntermediateNodePaymentTransaction::runPreviousNeighborRequestProcessingStage()
{
    debug() << "runPreviousNeighborRequestProcessingStage";
    const auto kNeighbor = mMessage->senderUUID;
    debug() << "Init. intermediate payment operation from node (" << kNeighbor << ")";
    debug() << "Requested amount reservation: " << mMessage->amount();


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
        maxNetworkDelay(3));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runCoordinatorRequestProcessingStage()
{
    debug() << "runCoordinatorRequestProcessingStage";
    if (! contextIsValid(Message::Payments_CoordinatorReservationRequest))
        return reject("No coordinator request received. Rolled back.");


    debug() << "Coordinator further reservation request received.";


    // TODO: add check for previous nodes amount reservation


    const auto kMessage = popNextMessage<CoordinatorReservationRequestMessage>();
    const auto kNextNode = kMessage->nextNodeInPathUUID();
    mCoordinator = kMessage->senderUUID;
    mLastProcessedPath = kMessage->pathUUID();

    // Note: copy of shared pointer is required
    const auto kOutgoingAmount = mTrustLines->availableOutgoingAmount(kNextNode);
    debug() << "requested reservation amount is " << kMessage->amount();
    debug() << "available outgoing amount to " << kNextNode << " is " << *kOutgoingAmount.get();
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

        debug() << "No amount reservation is possible. Rolled back.";
        rollBack(kMessage->pathUUID());
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
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
    debug() << "runNextNeighborResponseProcessingStage";
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        debug() << "No valid amount reservation response received. Rolled back.";
        rollBack(mLastProcessedPath);
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
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
        debug() << "Amount reservation rejected by the neighbor node.";
        rollBack(kMessage->pathUUID());
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    debug() << "(" << kContractor << ") accepted amount reservation.";

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
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runReservationProlongationStage()
{
    debug() << "runReservationProlongationStage";
    // on this stage we can receive IntermediateNodeReservationRequest message
    // and on this case we process PreviousNeighborRequestProcessing stage
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        mMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();
        return runPreviousNeighborRequestProcessingStage();
    }
    // In case if participants votes message is already received -
    // there is no need to prolong reservation, transaction may be proceeded.
    // Node is clarifying of coordinator if transaction is still alive
    if (!contextIsValid(Message::Payments_ParticipantsVotes)) {
        debug() << "Send TTLTransaction message to coordinator " << mCoordinator;
        sendMessage<TTLPolongationMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID());
        mStep = Stages::Common_ClarificationTransaction;
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
            Message::Payments_ParticipantsVotes,
            Message::Payments_TTLProlongation},
            maxNetworkDelay(2));
    }
    mStep = Stages::Common_VotesChecking;
    return runVotesCheckingStage();
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runClarificationOfTransaction()
{
    // on this stage we can receive IntermediateNodeReservationRequest and ParticipantsVotes messages
    // and on this cases we process it properly
    debug() << "runClarificationOfTransaction";
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        mMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();
        return runPreviousNeighborRequestProcessingStage();
    }
    if (contextIsValid(Message::MessageType::Payments_ParticipantsVotes, false)) {
        mStep = Stages::Common_VotesChecking;
        return runVotesCheckingStage();
    }
    if (!contextIsValid(Message::MessageType::Payments_TTLProlongation)) {
        return reject("No participants votes message received. Transaction was closed. Rolling Back");
    }
    // transactions is still alive and we continue waiting for messages
    debug() << "Transactions is still alive. Continue waiting for messages";
    mStep = Stages::IntermediateNode_ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes,
         Message::Payments_IntermediateNodeReservationRequest},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runVotesCheckingStageWithCoordinatorClarification()
{
    if (contextIsValid(Message::Payments_ParticipantsVotes, false)) {
        return runVotesCheckingStage();
    }
    debug() << "Send TTLTransaction message to coordinator " << mCoordinator;
    sendMessage<TTLPolongationMessage>(
        mCoordinator,
        currentNodeUUID(),
        currentTransactionUUID());
    mStep = Stages::Common_ClarificationTransaction;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_ParticipantsVotes,
         Message::Payments_TTLProlongation},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::approve()
{
    launchFourCyclesClosingTransactions();
    launchThreeCyclesClosingTransactions();
    BasePaymentTransaction::approve();
    return resultDone();
}

void IntermediateNodePaymentTransaction::launchFourCyclesClosingTransactions()
{
    vector<pair<NodeUUID, NodeUUID>> debtorsAndCreditorsFourCycles;
    map<PathUUID, NodeUUID> pathsReservations;
    for (const auto &itNodeUUIDAndReservations : mReservations) {
        for (const auto &itPathUUIDAndReservation : itNodeUUIDAndReservations.second) {
            auto pathReservation = pathsReservations.find(itPathUUIDAndReservation.first);
            if (pathReservation == pathsReservations.end()) {
                pathsReservations.insert(
                    make_pair(
                        itPathUUIDAndReservation.first,
                        itNodeUUIDAndReservations.first));
            } else {
                if (itPathUUIDAndReservation.second->direction() == AmountReservation::Outgoing) {
                    debtorsAndCreditorsFourCycles.push_back(
                        make_pair(
                            pathReservation->second,
                            itNodeUUIDAndReservations.first));
                } else if (itPathUUIDAndReservation.second->direction() == AmountReservation::Incoming) {
                    debtorsAndCreditorsFourCycles.push_back(
                        make_pair(
                            itNodeUUIDAndReservations.first,
                            pathReservation->second));
                }
            }
        }
    }
    for (auto const &debtorAndCreditor : debtorsAndCreditorsFourCycles) {
        const auto kTransaction = make_shared<CyclesFourNodesInitTransaction>(
            currentNodeUUID(),
            debtorAndCreditor.first,
            debtorAndCreditor.second,
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mLog);
        launchSubsidiaryTransaction(kTransaction);
    }
}

void IntermediateNodePaymentTransaction::launchThreeCyclesClosingTransactions()
{
    for (auto const nodeUUIDAndReservations : mReservations) {
        const auto kTransaction = make_shared<CyclesThreeNodesInitTransaction>(
            currentNodeUUID(),
            nodeUUIDAndReservations.first,
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mLog);
        launchSubsidiaryTransaction(kTransaction);
    }
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