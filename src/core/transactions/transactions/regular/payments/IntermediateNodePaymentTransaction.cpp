#include "IntermediateNodePaymentTransaction.h"


IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    const NodeUUID& currentNodeUUID,
    IntermediateNodeReservationRequestMessage::ConstShared message,
    TrustLinesManager* trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BasePaymentTransaction(
        BaseTransaction::IntermediateNodePaymentTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log,
        subsystemsController),
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
    Logger &log,
    SubsystemsController *subsystemsController) :

    BasePaymentTransaction(
        buffer,
        nodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log,
        subsystemsController)
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

            case Stages::IntermediateNode_ReservationProlongation:
                return runReservationProlongationStage();

            case Stages::Common_ClarificationTransactionBeforeVoting:
                return runClarificationOfTransactionBeforeVoting();

            case Stages::Common_VotesChecking:
                return runVotesCheckingStageWithCoordinatorClarification();

            case Stages::Common_ClarificationTransactionDuringVoting:
                return runClarificationOfTransactionDuringVoting();

            case Stages::Common_Recovery:
                return runVotesRecoveryParentStage();
            default:
                throw RuntimeError(
                    "IntermediateNodePaymentTransaction::run: "
                        "unexpected stage occurred.");
        }
    } catch (Exception &e) {
        error() << e.what();
        return recover("Something happens wrong in method run(). Transaction will be recovered");
    }
}


TransactionResult::SharedConst IntermediateNodePaymentTransaction::runPreviousNeighborRequestProcessingStage()
{
    debug() << "runPreviousNeighborRequestProcessingStage";
    const auto kNeighbor = mMessage->senderUUID;
    debug() << "Init. intermediate payment operation from node (" << kNeighbor << ")";

    if (mMessage->finalAmountsConfiguration().size() == 0) {
        error() << "Not received reservation";
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            0,                  // 0, because we don't know pathID
            ResponseMessage::Closed);
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    const auto kReservation = mMessage->finalAmountsConfiguration()[0];
    debug() << "Requested amount reservation: " << *kReservation.second.get() << " on path " << kReservation.first;
    debug() << "Received reservations size: " << mMessage->finalAmountsConfiguration().size();

    if (!mTrustLines->isNeighbor(kNeighbor)) {
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);
        error() << "Path is not valid: previous node is not neighbor of current one. Rejected.";
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        } else {
            mStep = Stages::IntermediateNode_ReservationProlongation;
            return resultWaitForMessageTypes(
                {Message::Payments_FinalAmountsConfiguration,
                 Message::Payments_IntermediateNodeReservationRequest,
                 Message::Payments_TTLProlongationResponse},
                maxNetworkDelay(kMaxPathLength - 2));
        }
    }

    // update local reservations during amounts from coordinator
    if (!updateReservations(vector<pair<PathID, ConstSharedTrustLineAmount>>(
        mMessage->finalAmountsConfiguration().begin() + 1,
        mMessage->finalAmountsConfiguration().end()))) {
        error() << "Previous node send path configuration, which is absent on current node";
        // next loop is only logger info
        for (const auto reservation : mMessage->finalAmountsConfiguration()) {
            debug() << "path: " << reservation.first << " amount: " << *reservation.second.get();
        }
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Closed);
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }
    debug() << "All reservations was updated";

    const auto kIncomingAmount = mTrustLines->availableIncomingAmount(kNeighbor);
    TrustLineAmount kReservationAmount =
            min(*kReservation.second.get(), *kIncomingAmount);

#ifdef TESTS
    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
        kNeighbor,
        kReservationAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    if (0 == kReservationAmount || ! reserveIncomingAmount(kNeighbor, kReservationAmount, kReservation.first)) {
        debug() << "No amount reservation is possible.";
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        } else {
            mStep = Stages::IntermediateNode_ReservationProlongation;
            return resultWaitForMessageTypes(
                {Message::Payments_FinalAmountsConfiguration,
                 Message::Payments_IntermediateNodeReservationRequest,
                 Message::Payments_TTLProlongationResponse},
                maxNetworkDelay(kMaxPathLength - 2));
        }
    }

    debug() << "reserve locally " << kReservationAmount << " to node " << kNeighbor << " on path " << kReservation.first;
    mLastReservedAmount = kReservationAmount;
    mLastProcessedPath = kReservation.first;
    sendMessage<IntermediateNodeReservationResponseMessage>(
        kNeighbor,
        currentNodeUUID(),
        currentTransactionUUID(),
        kReservation.first,
        ResponseMessage::Accepted,
        kReservationAmount);

    mStep = Stages::IntermediateNode_CoordinatorRequestProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationRequest,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(3));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runCoordinatorRequestProcessingStage()
{
    debug() << "runCoordinatorRequestProcessingStage";

    if (!contextIsValid(Message::Payments_CoordinatorReservationRequest, false)) {
        debug() << "No coordinator request received.";
        return runReservationProlongationStage();
    }

    debug() << "Coordinator further reservation request received.";

#ifdef TESTS
    mSubsystemsController->testThrowExceptionOnCoordinatorRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnCoordinatorRequestProcessingStage();
#endif
    // TODO: add check for previous nodes amount reservation

    const auto kMessage = popNextMessage<CoordinatorReservationRequestMessage>();
    const auto kNextNode = kMessage->nextNodeInPath();
    if (kMessage->finalAmountsConfiguration().size() == 0) {
        error() << "Not received reservation";
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            mLastProcessedPath,
            ResponseMessage::Closed);
        rollBack(mLastProcessedPath);
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    const auto kReservation = kMessage->finalAmountsConfiguration()[0];
    mCoordinator = kMessage->senderUUID;
    mLastProcessedPath = kReservation.first;

    debug() << "requested reservation amount is " << *kReservation.second.get() << " on path " << kReservation.first;
    debug() << "Next node is " << kNextNode;
    debug() << "Received reservations size: " << kMessage->finalAmountsConfiguration().size();
    if (!mTrustLines->isNeighbor(kNextNode)) {
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);
        debug() << "Path is not valid: next node is not neighbor of current one. Rolled back.";
        rollBack(kReservation.first);
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    // Note: copy of shared pointer is required
    const auto kOutgoingAmount = mTrustLines->availableOutgoingAmount(kNextNode);
    debug() << "available outgoing amount to " << kNextNode << " is " << *kOutgoingAmount.get();
    TrustLineAmount reservationAmount = min(
        *kReservation.second.get(),
        *kOutgoingAmount);

    if (0 == reservationAmount || ! reserveOutgoingAmount(kNextNode, reservationAmount, kReservation.first)) {
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);

        debug() << "No amount reservation is possible. Rolled back.";
        rollBack(kReservation.first);
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendRequestToIntNodeOnReservationStage(
        kNextNode,
        reservationAmount);
#endif

    debug() << "Reserve locally " << reservationAmount << " to node " << kNextNode << " on path " << kReservation.first;
    mLastReservedAmount = reservationAmount;

    // build reservation configuration for next node;
    // CoordinatorReservationRequestMessage contains configuration for next node
    vector<pair<PathID, ConstSharedTrustLineAmount>> reservations;
    reservations.push_back(
        make_pair(
            kReservation.first,
            make_shared<const TrustLineAmount>(reservationAmount)));

    if (kMessage->finalAmountsConfiguration().size() > 1) {
        // add actual reservations for next node
        reservations.insert(
            reservations.end(),
            kMessage->finalAmountsConfiguration().begin() + 1,
            kMessage->finalAmountsConfiguration().end());
    }
    debug() << "Prepared for sending reservations size: " << reservations.size();

    sendMessage<IntermediateNodeReservationRequestMessage>(
        kNextNode,
        currentNodeUUID(),
        currentTransactionUUID(),
        reservations);

    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_FinalPathConfiguration,
         Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runNextNeighborResponseProcessingStage()
{
    debug() << "runNextNeighborResponseProcessingStage";
    if (mContext.empty()) {
        debug() << "No amount reservation response received. Rolled back.";
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            mLastProcessedPath,
            ResponseMessage::NextNodeInaccessible);
        rollBack(mLastProcessedPath);
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    if (!contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        return runReservationProlongationStage();
    }

    const auto kMessage = popNextMessage<IntermediateNodeReservationResponseMessage>();
    const auto kContractor = kMessage->senderUUID;

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
        kContractor,
        kMessage->amountReserved());
    mSubsystemsController->testThrowExceptionOnNextNeighborResponseProcessingStage();
    mSubsystemsController->testTerminateProcessOnNextNeighborResponseProcessingStage();
#endif

    if (kMessage->state() == IntermediateNodeReservationResponseMessage::Closed) {
        // Receiver reject reservation and Coordinator should close transaction
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            mLastProcessedPath,
            ResponseMessage::Closed);
        debug() << "Amount reservation rejected with further transaction closing by the Receiver node.";
        rollBack(kMessage->pathID());
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    if (kMessage->state() == IntermediateNodeReservationResponseMessage::Rejected){
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            kMessage->pathID(),
            ResponseMessage::Rejected);
        debug() << "Amount reservation rejected by the neighbor node.";
        rollBack(kMessage->pathID());
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    debug() << "(" << kContractor << ") accepted amount reservation.";
    mLastReservedAmount = kMessage->amountReserved();

    shortageReservationsOnPath(
            kMessage->pathID(),
        kMessage->amountReserved());

#ifdef TESTS
    // coordinator wait for this message maxNetworkDelay(4)
    // we try get max delay with normal working algorithm on this stage
    mSubsystemsController->testSleepOnNextNeighborResponseProcessingStage(maxNetworkDelay(3));
#endif

    sendMessage<CoordinatorReservationResponseMessage>(
        mCoordinator,
        currentNodeUUID(),
        currentTransactionUUID(),
        kMessage->pathID(),
        ResponseMessage::Accepted,
        mLastReservedAmount);

    mStep = Stages::Common_FinalPathConfigurationChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalPathConfiguration,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runFinalPathConfigurationProcessingStage()
{
    // receive final amount on current path
    debug() << "runFinalPathConfigurationProcessingStage";

    if (!contextIsValid(Message::Payments_FinalPathConfiguration, false)) {
        debug() << "No final paths configuration was received from the coordinator.";
        return runReservationProlongationStage();
    }

    const auto kMessage = popNextMessage<FinalPathConfigurationMessage>();

    // TODO: check if message was really received from the coordinator;


    debug() << "Final payment path configuration received";

    // path was cancelled, drop all reservations belong it
    if (kMessage->amount() == 0) {
        rollBack(kMessage->pathID());
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
    }

    shortageReservationsOnPath(
            kMessage->pathID(),
        kMessage->amount());

    mStep = Stages::IntermediateNode_ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfiguration,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_TTLProlongationResponse},
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

    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        runClarificationOfTransactionBeforeVoting();
    }

    if (contextIsValid(Message::Payments_FinalPathConfiguration, false)) {
        runFinalPathConfigurationProcessingStage();
    }

    // Node is clarifying of coordinator if transaction is still alive
    if (!contextIsValid(Message::Payments_FinalAmountsConfiguration)) {
        debug() << "Send TTLTransaction message to coordinator " << mCoordinator;
        sendMessage<TTLProlongationRequestMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID());
        mStep = Stages::Common_ClarificationTransactionBeforeVoting;
        // we don't wait for Payments_FinalPathConfiguration message,
        // because it isn't critical for us
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
            Message::Payments_FinalAmountsConfiguration,
            Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(2));
    }
    return runFinalAmountsConfigurationConfirmation();
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runClarificationOfTransactionBeforeVoting()
{
    // on this stage we can receive IntermediateNodeReservationRequest, FinalAmountsConfiguration
    // messages and on this cases we process it properly
    debug() << "runClarificationOfTransactionBeforeVoting";
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        mMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();
        return runPreviousNeighborRequestProcessingStage();
    }

    if (contextIsValid(Message::Payments_FinalAmountsConfiguration, false)) {
        return runFinalAmountsConfigurationConfirmation();
    }

    if (!contextIsValid(Message::MessageType::Payments_TTLProlongationResponse)) {
        return reject("No actual message received. Transaction was closed. Rolling Back");
    }

    const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
    if (kMessage->state() == TTLProlongationResponseMessage::Continue) {
        // transactions is still alive and we continue waiting for messages
        debug() << "Transactions is still alive. Continue waiting for messages";
        mStep = Stages::IntermediateNode_ReservationProlongation;
        // we don't wait for Payments_FinalPathConfiguration message,
        // because it isn't critical for us
        return resultWaitForMessageTypes(
            {Message::Payments_TTLProlongationResponse,
             Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    return reject("Coordinator send response with transaction finish state. Rolling Back");
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runClarificationOfTransactionDuringVoting()
{
    debug() << "runClarificationOfTransactionDuringVoting";
    if (contextIsValid(Message::MessageType::Payments_ParticipantsVotes, false)) {
        mStep = Stages::Common_VotesChecking;
        return runVotesCheckingStage();
    }
    // this method is used also in voting stage, that's why we check if transaction is voted
    if (!contextIsValid(Message::MessageType::Payments_TTLProlongationResponse)) {
        if (mTransactionIsVoted) {
            return recover("No participants votes message with all votes received.");
        } else {
            return reject("No participants votes message received. Transaction was closed. Rolling Back");
        }
    }

    const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
    if (kMessage->state() == TTLProlongationResponseMessage::Continue) {
        // transactions is still alive and we continue waiting for messages
        debug() << "Transactions is still alive. Continue waiting for messages";
        mStep = Stages::Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    return reject("Coordinator send response with transaction finish state. Rolling Back");
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runFinalAmountsConfigurationConfirmation()
{
    // receive final configuration on all paths
    debug() << "runFinalAmountsConfigurationConfirmation";

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnFinalAmountClarificationStage();
#endif

    auto kMessage = popNextMessage<FinalAmountsConfigurationMessage>();
    if (!updateReservations(
        kMessage->finalAmountsConfiguration())) {
        sendMessage<FinalAmountsConfigurationResponseMessage>(
            kMessage->senderUUID,
            currentNodeUUID(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Rejected);
        return reject("There are some final amounts, reservations for which are absent. Rejected");
    }

#ifdef TESTS
    // coordinator wait for this message maxNetworkDelay(2)
    mSubsystemsController->testSleepOnFinalAmountClarificationStage(maxNetworkDelay(3));
#endif

    debug() << "All reservations was updated";
    sendMessage<FinalAmountsConfigurationResponseMessage>(
        kMessage->senderUUID,
        currentNodeUUID(),
        currentTransactionUUID(),
        FinalAmountsConfigurationResponseMessage::Accepted);

    mStep = Common_VotesChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(5)); // todo : need discuss this parameter (5)
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runVotesCheckingStageWithCoordinatorClarification()
{
    if (contextIsValid(Message::Payments_ParticipantsVotes, false)) {
        return runVotesCheckingStage();
    }

    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        debug() << "Receive TTL Message";
        const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
        if (kMessage->state() == TTLProlongationResponseMessage::Finish) {
            return reject("Coordinator send response with transaction finish state. Rolling Back");
        }
        return reject("Invalid TTL transaction state. Rolling back");
    }

    debug() << "Send TTLTransaction message to coordinator " << mCoordinator;
    sendMessage<TTLProlongationRequestMessage>(
        mCoordinator,
        currentNodeUUID(),
        currentTransactionUUID());
    mStep = Stages::Common_ClarificationTransactionDuringVoting;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

void IntermediateNodePaymentTransaction::shortageReservationsOnPath(
    const PathID pathID,
    const TrustLineAmount &amount)
{
    // Shortening all reservations that belongs to given path.
    for (const auto &nodeAndReservations : mReservations) {
        for (const auto &pathIDAndReservation : nodeAndReservations.second) {
            if (pathIDAndReservation.first == pathID) {
                shortageReservation(
                    nodeAndReservations.first,
                    pathIDAndReservation.second,
                    amount,
                    pathIDAndReservation.first);
            }
        }
    }
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::approve()
{
    mCommittedAmount = totalReservedAmount(
        AmountReservation::Outgoing);
    BasePaymentTransaction::approve();
    runBuildFourNodesCyclesSignal();
    runBuildThreeNodesCyclesSignal();
    return resultDone();
}

void IntermediateNodePaymentTransaction::runBuildFourNodesCyclesSignal()
{
    vector<pair<NodeUUID, NodeUUID>> debtorsAndCreditorsFourCycles;
    map<PathID, NodeUUID> pathsReservations;
    for (const auto &itNodeUUIDAndReservations : mReservations) {
        for (const auto &itPathIDAndReservation : itNodeUUIDAndReservations.second) {
            auto pathReservation = pathsReservations.find(itPathIDAndReservation.first);
            if (pathReservation == pathsReservations.end()) {
                pathsReservations.insert(
                    make_pair(
                        itPathIDAndReservation.first,
                        itNodeUUIDAndReservations.first));
            } else {
                if (itPathIDAndReservation.second->direction() == AmountReservation::Outgoing) {
                    debtorsAndCreditorsFourCycles.push_back(
                        make_pair(
                            pathReservation->second,
                            itNodeUUIDAndReservations.first));
                } else if (itPathIDAndReservation.second->direction() == AmountReservation::Incoming) {
                    debtorsAndCreditorsFourCycles.push_back(
                        make_pair(
                            itNodeUUIDAndReservations.first,
                            pathReservation->second));
                }
            }
        }
    }
    mBuildCycleFourNodesSignal(
        debtorsAndCreditorsFourCycles);
}

void IntermediateNodePaymentTransaction::runBuildThreeNodesCyclesSignal()
{
    vector<NodeUUID> contractorsUUID;
    contractorsUUID.reserve(mReservations.size());
    for (auto const nodeUUIDAndReservations : mReservations) {
        contractorsUUID.push_back(
            nodeUUIDAndReservations.first);
    }
    mBuildCycleThreeNodesSignal(
        contractorsUUID);
}

void IntermediateNodePaymentTransaction::savePaymentOperationIntoHistory(
    IOTransaction::Shared ioTransaction)
{
    debug() << "savePaymentOperationIntoHistory";
    ioTransaction->historyStorage()->savePaymentRecord(
        make_shared<PaymentRecord>(
            currentTransactionUUID(),
            PaymentRecord::PaymentOperationType::IntermediatePaymentType,
            mCommittedAmount));
    debug() << "Operation saved";
}

bool IntermediateNodePaymentTransaction::checkReservationsDirections() const
{
    debug() << "checkReservationsDirections";
    for (const auto nodeUUIDAndReservations : mReservations) {
        for (const auto pathIDAndReservation : nodeUUIDAndReservations.second) {
            const auto checkedPath = pathIDAndReservation.first;
            const auto checkedAmount = pathIDAndReservation.second->amount();
            int countIncomingReservations = 0;
            int countOutgoingReservations = 0;

            for (const auto nodeUUIDAndReservationsInternal : mReservations) {
                for (const auto pathIDAndReservationInternal : nodeUUIDAndReservationsInternal.second) {
                    if (pathIDAndReservationInternal.first == checkedPath) {
                        if (pathIDAndReservationInternal.second->amount() != checkedAmount) {
                            error() << "Amounts are different on path " << checkedPath;
                            return false;
                        }
                        if (pathIDAndReservationInternal.second->direction() == AmountReservation::Outgoing) {
                            countOutgoingReservations++;
                        }
                        if (pathIDAndReservationInternal.second->direction() == AmountReservation::Incoming) {
                            countIncomingReservations++;
                        }
                    }
                }
            }

            if (countIncomingReservations != 1 || countOutgoingReservations != 1) {
                error() << "Count incoming and outgoing reservations are invalid on path " << checkedPath;
                return false;
            }
        }
    }
    return true;
}

const string IntermediateNodePaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[IntermediateNodePaymentTA: " << currentTransactionUUID() << "] ";
    return s.str();
}