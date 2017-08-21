#include "CoordinatorPaymentTransaction.h"

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    const NodeUUID &kCurrentNodeUUID,
    const CreditUsageCommand::Shared kCommand,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    ResourcesManager *resourcesManager,
    Logger &log,
    SubsystemsController *subsystemsController)
noexcept :

    BasePaymentTransaction(
        BaseTransaction::CoordinatorPaymentTransaction,
        kCurrentNodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log,
        subsystemsController),
    mCommand(kCommand),
    mResourcesManager(resourcesManager),
    mReservationsStage(0),
    mDirectPathIsAllreadyProcessed(false)
{
    mStep = Stages::Coordinator_Initialisation;
}

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    ResourcesManager *resourcesManager,
    Logger &log,
    SubsystemsController *subsystemsController)
throw (bad_alloc) :

    BasePaymentTransaction(
        buffer,
        nodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log,
        subsystemsController),
    mResourcesManager(resourcesManager)
{}


TransactionResult::SharedConst CoordinatorPaymentTransaction::run()
    noexcept
{
    while (true) {
        debug() << "run: stage: " << mStep;
        try {
            switch (mStep) {
                case Stages::Coordinator_Initialisation:
                    return runPaymentInitialisationStage();

                case Stages::Coordinator_ReceiverResourceProcessing:
                    return runReceiverResourceProcessingStage();

                case Stages::Coordinator_ReceiverResponseProcessing:
                    return runReceiverResponseProcessingStage();

                case Stages::Coordinator_AmountReservation:
                    return runAmountReservationStage();

                case Stages::Coordinator_ShortPathAmountReservationResponseProcessing:
                    return runDirectAmountReservationResponseProcessingStage();

                case Stages::Coordinator_FinalAmountsConfigurationConfirmation:
                    return runFinalAmountsConfigurationConfirmation();

                case Stages::Common_VotesChecking:
                    return runVotesConsistencyCheckingStage();

                case Stages::Common_Recovery:
                    return runVotesRecoveryParentStage();

                default:
                    throw RuntimeError(
                            "CoordinatorPaymentTransaction::run(): "
                                    "invalid transaction step.");
            }
        } catch (CallChainBreakException &e) {
            error() << e.what();
            // on this case we break call functions chain and prevent stack overflow
            mReservationsStage = 2;
            continue;
        } catch (Exception &e) {
            error() << e.what();
            return reject("Something happens wrong in method run(). Transaction will be rejected");
        }
    }
}


TransactionResult::SharedConst CoordinatorPaymentTransaction::runPaymentInitialisationStage ()
{
    debug() << "Operation initialised to the node (" << mCommand->contractorUUID() << ")";
    debug() << "Command UUID: " << mCommand->UUID();
    debug() << "Operation amount: " << mCommand->amount();

    if (mCommand->contractorUUID() == currentNodeUUID()) {
        debug() << "Attempt to initialise operation against itself was prevented. Canceled.";
        return resultProtocolError();
    }


    // Check if total outgoing possibilities of this node are not smaller,
    // than total operation amount. In case if so - there is no reason to begin the operation:
    // current node would not be able to pay such an amount.
    const auto kTotalOutgoingPossibilities = *(mTrustLines->totalOutgoingAmount());
    if (kTotalOutgoingPossibilities < mCommand->amount())
        return resultInsufficientFundsError();

    // TODO: Ensure paths shuffling

    NodeUUID sender = currentNodeUUID();

    mResourcesManager->requestPaths(
        currentTransactionUUID(),
        mCommand->contractorUUID());

    mStep = Stages::Coordinator_ReceiverResourceProcessing;
    return transactionResultFromState(
        TransactionState::waitForResourcesTypes(
            {BaseResource::ResourceType::Paths},
            maxNetworkDelay(4)));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runReceiverResourceProcessingStage()
{
    debug() << "runReceiverResourceProcessingStage";
    if (mResources.size() != 0) {
        auto responseResource = *mResources.begin();
        if (responseResource->type() == BaseResource::ResourceType::Paths) {

            PathsResource::Shared response = static_pointer_cast<PathsResource>(
                responseResource);
            response->pathCollection()->resetCurrentPath();
            while (response->pathCollection()->hasNextPath()) {
                auto path = response->pathCollection()->nextPath();
                if (isPathValid(path)) {
                    addPathForFurtherProcessing(path);
                }
            }
        } else {
            throw Exception("CoordinatorPaymentTransaction::runReceiverResourceProcessingStage: "
                                "unexpected resource type");
        }
    } else {
        error() << "resources are empty";
        return resultNoPathsError();
    }

    // If there is no one path to the receiver - transaction can't proceed.
    if (mPathsStats.empty())
        return resultNoPathsError();

    debug() << "Collected paths count: " << mPathsStats.size();

    // Sending message to the receiver note to approve the payment receiving.
    sendMessage<ReceiverInitPaymentRequestMessage>(
        mCommand->contractorUUID(),
        currentNodeUUID(),
        currentTransactionUUID(),
        mCurrentAmountReservingPathIdentifier,
        mCommand->amount());

    mStep = Stages::Coordinator_ReceiverResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_ReceiverInitPaymentResponse},
        kMaxMessageTransferLagMSec);
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runReceiverResponseProcessingStage ()
{
    if (! contextIsValid(Message::Payments_ReceiverInitPaymentResponse))
        return exitWithResult(
            resultNoResponseError(),
            "Receiver reservation response wasn't received. Canceling.");

    const auto kMessage = popNextMessage<ReceiverInitPaymentResponseMessage>();
    if (kMessage->state() != ReceiverInitPaymentResponseMessage::Accepted) {
        return exitWithResult(
            resultInsufficientFundsError(),
            "Receiver rejected payment operation. Canceling.");
    }

    debug() << "Receiver accepted operation. Begin reserving amounts.";
    mStep = Stages::Coordinator_AmountReservation;
    return runAmountReservationStage();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runAmountReservationStage ()
{
    debug() << "runAmountReservationStage";
    switch (mReservationsStage) {
    case 0: {
        initAmountsReservationOnNextPath();
        mReservationsStage += 1;

        // Note:
        // next section must be executed immediately.
        // (no "break" is needed).
        }

    case 1: {
        // nodes can clarify if transaction is still alive
        if (contextIsValid(Message::MessageType::Payments_TTLProlongationRequest, false)) {
            return runTTLTransactionResponce();
        }
        const auto kPathStats = currentAmountReservationPathStats();
        if (kPathStats->path()->length() == 2) {
            // In case if path contains only sender and receiver -
            // middleware nodes reservation must be omitted.
            return tryReserveAmountDirectlyOnReceiver(
                mCurrentAmountReservingPathIdentifier,
                kPathStats);
        }

        else if (kPathStats->isReadyToSendNextReservationRequest())
            return tryReserveNextIntermediateNodeAmount(kPathStats);

        else if (kPathStats->isWaitingForNeighborReservationResponse())
            return processNeighborAmountReservationResponse();

        else if (kPathStats->isWaitingForNeighborReservationPropagationResponse())
            return processNeighborFurtherReservationResponse();

        else if (kPathStats->isWaitingForReservationResponse())
            return processRemoteNodeResponse();

        throw RuntimeError(
            "CoordinatorPaymentTransaction::processAmountReservationStage: "
                "unexpected behaviour occurred.");
        }

    case 2:
        mReservationsStage = 1;
        return tryProcessNextPath();

    default:
        throw ValueError(
            "CoordinatorPaymentTransaction::processAmountReservationStage: "
            "unexpected reservations stage occurred.");
    }
}

/**
 * @brief CoordinatorPaymentTransaction::propagateVotesList
 * Collects all nodes from all paths into one votes list,
 * and propagates it to the next node in the votes list.
 * @param shouldSetUpDelay flag which tell us if need check on delay before sending,
 * has true value only on processRemoteNodeResponse stage
 */
TransactionResult::SharedConst CoordinatorPaymentTransaction::propagateVotesListAndWaitForVoutingResult()
{
    debug() << "propagateVotesListAndWaitForVoutingResult";
    const auto kCurrentNodeUUID = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();

    // TODO: additional check if payment is correct

    // Prevent simple transaction rolling back
    // todo: make this atomic
    mTransactionIsVoted = true;

    mParticipantsVotesMessage = make_shared<ParticipantsVotesMessage>(
        kCurrentNodeUUID,
        kTransactionUUID,
        kCurrentNodeUUID);

    for (PathUUID pathIdx = 0; pathIdx <= mCurrentAmountReservingPathIdentifier; pathIdx++) {
        auto const pathStats = mPathsStats[pathIdx].get();

        if (pathStats->path()->length() > 2)
            if (!pathStats->isLastIntermediateNodeApproved())
                continue;

        for (const auto &nodeUUID : pathStats->path()->nodes) {
            // By the protocol, coordinator node must be excluded from the message.
            // Only coordinator may emit ParticipantsApprovingMessage into the network.
            // It is supposed, that in case if it was emitted - than coordinator approved the transaction.
            //
            // TODO: [mvp] [cryptography] despite this, coordinator must sign the message,
            // so the other nodes would be possible to know that this message was emitted by the coordinator.
            if (nodeUUID == kCurrentNodeUUID)
                continue;

            mParticipantsVotesMessage->addParticipant(nodeUUID);
        }
    }


#ifdef DEBUG
    debug() << "Total participants included: " << mParticipantsVotesMessage->participantsCount();
    debug() << "Participants order is the next:";
    for (const auto kNodeUUIDAndVote : mParticipantsVotesMessage->votes()) {
        debug() << kNodeUUIDAndVote.first;
    }
#endif

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToNextNodeOnVoteStage();
#endif

    // Begin message propagation
    sendMessage(
        mParticipantsVotesMessage->firstParticipant(),
        mParticipantsVotesMessage);

    debug() << "Votes message constructed and sent to the (" << mParticipantsVotesMessage->firstParticipant() << ")";

#ifdef TESTS
    mSubsystemsController->testThrowExceptionOnVoteStage();
    mSubsystemsController->testTerminateProcessOnVoteStage();
#endif

    mStep = Stages::Common_VotesChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes,
        Message::Payments_TTLProlongationRequest},
        maxNetworkDelay(mParticipantsVotesMessage->participantsCount() + 1));
}


void CoordinatorPaymentTransaction::initAmountsReservationOnNextPath()
{
    if (mPathsStats.empty())
        throw NotFoundError(
            "CoordinatorPaymentTransaction::tryBlockAmounts: "
            "no paths are available.");

    mCurrentAmountReservingPathIdentifier = *mPathUUIDs.cbegin();
    debug() << "[" << mCurrentAmountReservingPathIdentifier << "] {"
            << currentAmountReservationPathStats()->path()->toString() << "}";
}

/*
 * Tries to reserve amount on path that consists only of sender and receiver nodes.
 */
TransactionResult::SharedConst CoordinatorPaymentTransaction::tryReserveAmountDirectlyOnReceiver (
    const PathUUID pathUUID,
    PathStats *pathStats)
{
    debug() << "tryReserveAmountDirectlyOnReceiver";
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(pathStats->path()->length() == 2);
#endif

    if (mDirectPathIsAllreadyProcessed) {
        error() << "Direct path reservation attempt occurred, but previously it was already processed. "
                << "It seems that paths collection contains direct path several times. "
                << "This one and all other similar path would be rejected. "
                << "Switching to the other path.";

        pathStats->setUnusable();
        return tryProcessNextPath();
    }
    mDirectPathIsAllreadyProcessed = true;


    debug() << "Direct path occurred (coordinator -> receiver). "
           << "Trying to reserve amount directly on the receiver side.";


    const auto kCoordinator = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();
    const auto kContractor = mCommand->contractorUUID();


    // ToDo: implement operator < for TrustLineAmount and remove this pure conversion

    // Check if local reservation is possible.
    // If not - there is no reason to send any reservations requests.
    const auto kAvailableOutgoingAmount = mTrustLines->availableOutgoingAmount(kContractor);
    if (*kAvailableOutgoingAmount == TrustLineAmount(0)) {
        debug() << "There is no direct outgoing amount available for the receiver node. "
               << "Switching to another path.";

        pathStats->setUnusable();
        return tryProcessNextPath();
    }

    // Note: try reserve remaining part of command amount
    const auto kRemainingAmountForProcessing =
            mCommand->amount() - totalReservedAmount(AmountReservation::Outgoing);
    // Reserving amount locally.
    const auto kReservationAmount = min(kRemainingAmountForProcessing, *kAvailableOutgoingAmount);
    if (not reserveOutgoingAmount(
        kContractor,
        kReservationAmount,
        pathUUID)){
        error() << "Can't reserve amount locally. "
                << "Switching to another path.";

        pathStats->setUnusable();
        return tryProcessNextPath();
    }

    // Reserving on the contractor side
    pathStats->shortageMaxFlow(kReservationAmount);
    vector<pair<PathUUID, ConstSharedTrustLineAmount>> reservations;
    reservations.push_back(
        make_pair(
            mCurrentAmountReservingPathIdentifier,
            make_shared<const TrustLineAmount>(kReservationAmount)));

    if (mNodesFinalAmountsConfiguration.find(kContractor) != mNodesFinalAmountsConfiguration.end()) {
        // add existing contractor reservations
        const auto kContractorReservations = mNodesFinalAmountsConfiguration[kContractor];
        reservations.insert(
            reservations.end(),
            kContractorReservations.begin(),
            kContractorReservations.end());
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToReceiverOnReservationStage();
#endif

    debug() << "Send reservations size: " << reservations.size();
    sendMessage<IntermediateNodeReservationRequestMessage>(
        kContractor,
        kCoordinator,
        kTransactionUUID,
        reservations);

    debug() << "Reservation request for " << kReservationAmount << " sent directly to the receiver node.";

    mStep = Stages::Coordinator_ShortPathAmountReservationResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse},
        maxNetworkDelay(2));
}


TransactionResult::SharedConst CoordinatorPaymentTransaction::tryReserveNextIntermediateNodeAmount (
    PathStats *pathStats)
{
    debug() << "tryReserveNextIntermediateNodeAmount";
    /*
     * Nodes scheme:
     *  R - remote node;
     *  S - next node in path after remote one;
     */

    try {
        const auto R_UUIDAndPos = pathStats->nextIntermediateNodeAndPos();
        const auto R_UUID = R_UUIDAndPos.first;
        const auto R_PathPosition = R_UUIDAndPos.second;

        const auto S_PathPosition = R_PathPosition + 1;
        const auto S_UUID = pathStats->path()->nodes[S_PathPosition];

        if (R_PathPosition == 1) {
            if (pathStats->isNeighborAmountReserved())
                return askNeighborToApproveFurtherNodeReservation(
                    R_UUID,
                    pathStats);

            else
                return askNeighborToReserveAmount(
                    R_UUID,
                    pathStats);

        } else {
            debug() << "Processing " << int(R_PathPosition) << " node in path: (" << R_UUID << ").";

            return askRemoteNodeToApproveReservation(
                pathStats,
                R_UUID,
                R_PathPosition,
                S_UUID);
        }

    } catch(NotFoundError) {
        debug() << "No unprocessed paths are left.";
        debug() << "Requested amount can't be collected. Canceling.";
        rollBack();
        return resultInsufficientFundsError();
    }
}

void CoordinatorPaymentTransaction::addPathForFurtherProcessing(
    Path::ConstShared path)
{
    // Preventing paths duplication
    for (const auto &identifierAndStats : mPathsStats) {
        if (identifierAndStats.second->path() == path)
            throw ConflictError("CoordinatorPaymentTransaction::addPathForFurtherProcessing: "
                "duplicated path occured in the transaction.");
    }

    PathUUID currentPathUUID = 0;
    for (;;) {
        // Cycle is needed to prevent possible hashes collision.
        PathUUID identifier = currentPathUUID++;// boost::uuids::random_generator()();
        if (mPathsStats.count(identifier) == 0){
            mPathsStats[identifier] = make_unique<PathStats>(path);
            mPathUUIDs.push_back(identifier);
            return;
        }
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askNeighborToReserveAmount(
    const NodeUUID &neighbor,
    PathStats *path)
{
    debug() << "askNeighborToReserveAmount";
    const auto kCurrentNode = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();

    if (! mTrustLines->isNeighbor(neighbor)){
        // Internal process error.
        // No next path must be selected.
        // Transaction execution must be cancelled.

        error() << "Invalid path occurred. Node (" << neighbor << ") is not listed in first level contractors list.";
        error() << "This may signal about protocol/data manipulations.";

        throw RuntimeError(
            "CoordinatorPaymentTransaction::tryReserveNextIntermediateNodeAmount: "
            "invalid first level node occurred. ");
    }

    // Note: copy of shared pointer is required
    const auto kAvailableOutgoingAmount =  mTrustLines->availableOutgoingAmount(neighbor);
    // Note: try reserve remaining part of command amount
    const auto kRemainingAmountForProcessing =
            mCommand->amount() - totalReservedAmount(AmountReservation::Outgoing);

    const auto kReservationAmount = min(*kAvailableOutgoingAmount, kRemainingAmountForProcessing);

    if (kReservationAmount == 0) {
        debug() << "AvailableOutgoingAmount " << *kAvailableOutgoingAmount;
        debug() << "RemainingAmountForProcessing " << kRemainingAmountForProcessing;
        debug() << "No payment amount is available for (" << neighbor << "). "
                  "Switching to another path.";

        path->setUnusable();
        throw CallChainBreakException("Break call chain for preventing call loop");
    }

    if (not reserveOutgoingAmount(
        neighbor,
        kReservationAmount,
        mCurrentAmountReservingPathIdentifier)) {
        error() << "Can't reserve amount locally. "
                << "Switching to another path.";
        path->setUnusable();
        throw CallChainBreakException("Break call chain for preventing call loop");
    }

    // Try reserve amount locally.
    path->shortageMaxFlow(kReservationAmount);
    path->setNodeState(
        1,
        PathStats::NeighbourReservationRequestSent);

    vector<pair<PathUUID, ConstSharedTrustLineAmount>> reservations;
    reservations.push_back(
        make_pair(
            mCurrentAmountReservingPathIdentifier,
            make_shared<const TrustLineAmount>(kReservationAmount)));

    if (mNodesFinalAmountsConfiguration.find(neighbor) != mNodesFinalAmountsConfiguration.end()) {
        // add existing neighbor reservations
        const auto kNeighborReservations = mNodesFinalAmountsConfiguration[neighbor];
        reservations.insert(
            reservations.end(),
            kNeighborReservations.begin(),
            kNeighborReservations.end());
    }
    debug() << "Prepared for sending reservations size: " << reservations.size();

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToReceiverOnReservationStage();
#endif

    sendMessage<IntermediateNodeReservationRequestMessage>(
        neighbor,
        kCurrentNode,
        kTransactionUUID,
        reservations);

    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse,
        Message::Payments_TTLProlongationRequest},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askNeighborToApproveFurtherNodeReservation(
    const NodeUUID& neighbor,
    PathStats *path)
{
    debug() << "askNeighborToApproveFurtherNodeReservation";
    const auto kCoordinator = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();
    const auto kNeighborPathPosition = 1;
    const auto kNextAfterNeighborNode = path->path()->nodes[kNeighborPathPosition+1];

    // Note:
    // no check of "neighbor" node is needed here.
    // It was done on previous step.

    vector<pair<PathUUID, ConstSharedTrustLineAmount>> reservations;
    reservations.push_back(
        make_pair(
            mCurrentAmountReservingPathIdentifier,
            make_shared<const TrustLineAmount>(path->maxFlow())));

    if (mNodesFinalAmountsConfiguration.find(kNextAfterNeighborNode) != mNodesFinalAmountsConfiguration.end()) {
        // add existing next after neighbor node reservations
        const auto kNeighborReservations = mNodesFinalAmountsConfiguration[kNextAfterNeighborNode];

        reservations.insert(
            reservations.end(),
            kNeighborReservations.begin(),
            kNeighborReservations.end());
    }
    debug() << "Prepared for sending reservations size: " << reservations.size();

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
        neighbor,
        path->maxFlow());
#endif

    sendMessage<CoordinatorReservationRequestMessage>(
        neighbor,
        kCoordinator,
        kTransactionUUID,
        reservations,
        kNextAfterNeighborNode);

    debug() << "Further amount reservation request sent to the node (" << neighbor << ") ["
            << path->maxFlow() << "]" << ", next node - (" << kNextAfterNeighborNode << ")";

    path->setNodeState(
        kNeighborPathPosition,
        PathStats::ReservationRequestSent);


    // delay is equal 4 because in IntermediateNodePaymentTransaction::runCoordinatorRequestProcessingStage delay is 2
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationResponse,
        Message::Payments_TTLProlongationRequest},
        maxNetworkDelay(4));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processNeighborAmountReservationResponse()
{
    debug() << "processNeighborAmountReservationResponse";
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        debug() << "No neighbor node response received. Switching to another path.";
        // dropping reservation to first node
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);

        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mCommand->contractorUUID(),
            currentNodeUUID(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);
        return tryProcessNextPath();
    }


    auto message = popNextMessage<IntermediateNodeReservationResponseMessage>();
    // todo: check message sender

    if (message->state() == IntermediateNodeReservationResponseMessage::Closed) {
        error() << "Neighbor node doesn't approved reservation request";
        return reject("Desynchronization in reservation with Receiver occured. Transaction closed.");
    }

    if (message->state() == IntermediateNodeReservationResponseMessage::Rejected) {
        debug() << "Neighbor node doesn't approved reservation request";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        return tryProcessNextPath();
    }


    debug() << "(" << message->senderUUID << ") approved reservation request.";
    auto path = currentAmountReservationPathStats();
    path->setNodeState(
        1, PathStats::NeighbourReservationApproved);
    path->shortageMaxFlow(message->amountReserved());

    // shortage reservation
    // TODO maybe add if change path->maxFlow()
    auto nodeReservations = mReservations[message->senderUUID];
    for (const auto pathUUIDAndreservation : nodeReservations) {
        if (pathUUIDAndreservation.first == mCurrentAmountReservingPathIdentifier) {
            shortageReservation(
                message->senderUUID,
                pathUUIDAndreservation.second,
                path->maxFlow(),
                mCurrentAmountReservingPathIdentifier);
        }
    }

    return runAmountReservationStage();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processNeighborFurtherReservationResponse()
{
    debug() << "processNeighborFurtherReservationResponse";
    if (! contextIsValid(Message::Payments_CoordinatorReservationResponse)) {
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mCommand->contractorUUID(),
            currentNodeUUID(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);
        debug() << "Switching to another path.";
        return tryProcessNextPath();
    }

    auto message = popNextMessage<CoordinatorReservationResponseMessage>();

    if (message->state() == CoordinatorReservationResponseMessage::Closed) {
        return reject("Desynchronization in reservation with Receiver occured. Transaction closed.");
    }

    if (message->amountReserved() == 0 || message->state() == CoordinatorReservationResponseMessage::Rejected) {
        debug() << "Neighbor node doesn't accepted coordinator request.";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mCommand->contractorUUID(),
            currentNodeUUID(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);
        return tryProcessNextPath();
    }

    auto path = currentAmountReservationPathStats();
    path->setNodeState(
        1,
        PathStats::ReservationApproved);
    debug() << "Neighbor node accepted coordinator request. Reserved: " << message->amountReserved();

    path->shortageMaxFlow(message->amountReserved());
    debug() << "Path max flow is now " << path->maxFlow();

    // shortage reservation
    // TODO maybe add if change path->maxFlow()
    auto nodeReservations = mReservations[message->senderUUID];
    for (const auto pathUUIDAndreservation : nodeReservations) {
        if (pathUUIDAndreservation.first == mCurrentAmountReservingPathIdentifier) {
            shortageReservation(
                message->senderUUID,
                pathUUIDAndreservation.second,
                path->maxFlow(),
                mCurrentAmountReservingPathIdentifier);
        }
    }


    if (path->isLastIntermediateNodeProcessed()) {

        const auto kTotalAmount = totalReservedAmount(
            AmountReservation::Outgoing);

        debug() << "Current path reservation finished";
        debug() << "Total collected amount by all paths: " << kTotalAmount;

        if (kTotalAmount > mCommand->amount()){
            error() << "Total requested amount: " << mCommand->amount();
            error() << "Total collected amount is greater than requested amount. "
                "It indicates that some of the nodes doesn't follows the protocol, "
                "or that an error is present in protocol itself.";
            rollBack();
            return resultNoConsensusError();
        }

#ifdef TESTS
        mSubsystemsController->testForbidSendMessageWithFinalPathConfiguration(
            path->path()->intermediateUUIDs().size());
#endif

        // send final path amount to all intermediate nodes on path
        sendFinalPathConfiguration(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier,
            path->maxFlow());

        addFinalConfigurationOnPath(
            mCurrentAmountReservingPathIdentifier,
            path);

        if (kTotalAmount == mCommand->amount()){
            debug() << "Total requested amount: " << mCommand->amount() << ". Collected.";

            return sendFinalAmountsConfigurationToAllParticipants();
        }
        return tryProcessNextPath();
    }

    return runAmountReservationStage();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askRemoteNodeToApproveReservation(
    PathStats* path,
    const NodeUUID& remoteNode,
    const byte remoteNodePosition,
    const NodeUUID& nextNodeAfterRemote)
{
    debug() << "askRemoteNodeToApproveReservation";
    const auto kCoordinator = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();

    vector<pair<PathUUID, ConstSharedTrustLineAmount>> reservations;
    reservations.push_back(
        make_pair(
            mCurrentAmountReservingPathIdentifier,
            make_shared<const TrustLineAmount>(path->maxFlow())));

    if (mNodesFinalAmountsConfiguration.find(nextNodeAfterRemote) != mNodesFinalAmountsConfiguration.end()) {
        // add existing next after remote node reservations
        const auto kNeighborReservations = mNodesFinalAmountsConfiguration[nextNodeAfterRemote];
        reservations.insert(
            reservations.end(),
            kNeighborReservations.begin(),
            kNeighborReservations.end());
    }

    debug() << "Prepared for sending reservations size: " << reservations.size();

#ifdef TESTS
    mSubsystemsController->testForbidSendRequestToIntNodeOnReservationStage(
        remoteNode,
        path->maxFlow());
#endif

    sendMessage<CoordinatorReservationRequestMessage>(
        remoteNode,
        kCoordinator,
        kTransactionUUID,
        reservations,
        nextNodeAfterRemote);

    path->setNodeState(
        remoteNodePosition,
        PathStats::ReservationRequestSent);

    debug() << "Further amount reservation request sent to the node (" << remoteNode << ") ["
           << path->maxFlow() << ", next node - (" << nextNodeAfterRemote << ")]";

    // delay is equal 4 because in IntermediateNodePaymentTransaction::runCoordinatorRequestProcessingStage delay is 2
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationResponse,
        Message::Payments_TTLProlongationRequest},
        maxNetworkDelay(4));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processRemoteNodeResponse()
{
    debug() << "processRemoteNodeResponse";
    if (! contextIsValid(Message::Payments_CoordinatorReservationResponse)){
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mCommand->contractorUUID(),
            currentNodeUUID(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);
        debug() << "Switching to another path.";
        return tryProcessNextPath();
    }


    const auto message = popNextMessage<CoordinatorReservationResponseMessage>();
    auto path = currentAmountReservationPathStats();

    /*
     * Nodes scheme:
     * R - remote node;
     */

    const auto R_UUIDAndPos = path->currentIntermediateNodeAndPos();
    const auto R_PathPosition = R_UUIDAndPos.second;

    if (message->state() == CoordinatorReservationResponseMessage::Closed) {
        return reject("Desynchronization in reservation with Receiver occured. Transaction closed.");
    }

    if (0 == message->amountReserved() || message->state() == CoordinatorReservationResponseMessage::Rejected) {
        debug() << "Remote node rejected reservation. Switching to another path.";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mCommand->contractorUUID(),
            currentNodeUUID(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);

        path->setUnusable();
        path->setNodeState(
            R_PathPosition,
            PathStats::ReservationRejected);

        return tryProcessNextPath();

    } else {
        const auto reservedAmount = message->amountReserved();

        path->shortageMaxFlow(reservedAmount);
        path->setNodeState(
            R_PathPosition,
            PathStats::ReservationApproved);

        // shortage reservation
        // TODO maybe add if change path->maxFlow()
        auto firstIntermediateNode = path->path()->nodes[1];
        auto nodeReservations = mReservations[firstIntermediateNode];
        for (auto const pathUUIDAndReservation : nodeReservations) {
            if (pathUUIDAndReservation.first == mCurrentAmountReservingPathIdentifier) {
                shortageReservation(
                    firstIntermediateNode,
                    pathUUIDAndReservation.second,
                    path->maxFlow(),
                    mCurrentAmountReservingPathIdentifier);
            }
        }

        debug() << "(" << message->senderUUID << ") reserved " << reservedAmount;
        debug() << "Path max flow is now " << path->maxFlow();

        if (path->isLastIntermediateNodeProcessed()) {

            const auto kTotalAmount = totalReservedAmount(
                AmountReservation::Outgoing);

            debug() << "Current path reservation finished";
            debug() << "Total collected amount by all paths: " << kTotalAmount;

            if (kTotalAmount > mCommand->amount()){
                error() << "Total requested amount: " << mCommand->amount();
                error() << "Total collected amount is greater than requested amount. "
                           "It indicates that some of the nodes doesn't follows the protocol, "
                           "or that an error is present in protocol itself.";
                rollBack();
                return resultNoConsensusError();
            }

#ifdef TESTS
            mSubsystemsController->testForbidSendMessageWithFinalPathConfiguration(
                path->path()->intermediateUUIDs().size());
#endif

            // send final path amount to all intermediate nodes on path
            sendFinalPathConfiguration(
                currentAmountReservationPathStats(),
                mCurrentAmountReservingPathIdentifier,
                path->maxFlow());

            addFinalConfigurationOnPath(
                mCurrentAmountReservingPathIdentifier,
                path);

            if (kTotalAmount == mCommand->amount()){
                debug() << "Total requested amount: " << mCommand->amount() << ". Collected.";

                return sendFinalAmountsConfigurationToAllParticipants();
            }
            return tryProcessNextPath();
        }

        return tryReserveNextIntermediateNodeAmount(path);
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::tryProcessNextPath()
{
    debug() << "tryProcessNextPath";
    try {
        switchToNextPath();
        return runAmountReservationStage();

    } catch (NotFoundError &e) {
        debug() << "No another paths are available. Canceling.";
        rollBack();
        return resultInsufficientFundsError();
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::sendFinalAmountsConfigurationToAllParticipants()
{
    debug() << "sendFinalAmountsConfigurationToAllParticipants";

    mFinalAmountNodesConfirmation.clear();

    // check if reservation to contractor present
    const auto contractorNodeReservations = mReservations.find(mCommand->contractorUUID());
    if (contractorNodeReservations != mReservations.end()) {
        if (contractorNodeReservations->second.size() > 1) {
            return reject("Coordinator has more than one reservation to contractor");
        }
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnFinalAmountClarificationStage();
#endif

    for (auto const nodeAndFinalAmountsConfig : mNodesFinalAmountsConfiguration) {
        // todo : discuss if receiver need on final amounts
        if (nodeAndFinalAmountsConfig.first == mCommand->contractorUUID()) {
            continue;
        }
        debug() << "send final amount configuration to " << nodeAndFinalAmountsConfig.first;
        sendMessage<FinalAmountsConfigurationMessage>(
            nodeAndFinalAmountsConfig.first,
            currentNodeUUID(),
            currentTransactionUUID(),
            nodeAndFinalAmountsConfig.second);
        mFinalAmountNodesConfirmation.insert(
            make_pair(
                nodeAndFinalAmountsConfig.first,
                false));
    }

    debug() << "Total count of all participants without coordinator is " << mNodesFinalAmountsConfiguration.size();

    mStep = Coordinator_FinalAmountsConfigurationConfirmation;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfigurationResponse,
         Message::Payments_TTLProlongationRequest},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runFinalAmountsConfigurationConfirmation()
{
    debug() << "runFinalAmountsConfigurationConfirmation";
    if (contextIsValid(Message::MessageType::Payments_TTLProlongationRequest, false)) {
        return runTTLTransactionResponce();
    }
    if (contextIsValid(Message::MessageType::Payments_FinalAmountsConfigurationResponse, false)) {
        auto kMessage = popNextMessage<FinalAmountsConfigurationResponseMessage>();
        if (mFinalAmountNodesConfirmation.find(kMessage->senderUUID) == mFinalAmountNodesConfirmation.end()) {
            // todo : need actual action on this case
            error() << "Sender is not participant of this transaction";
            return resultWaitForMessageTypes(
                {Message::Payments_FinalAmountsConfigurationResponse,
                 Message::Payments_TTLProlongationRequest},
                maxNetworkDelay(2));
        }
        if (kMessage->state() == FinalAmountsConfigurationResponseMessage::Rejected) {
            debug() << "Node " << kMessage->senderUUID << " rejected final amounts";
            return reject("Haven't reach consensus on reservation. Transaction rejected.");
        }
        debug() << "Node " << kMessage->senderUUID << " confirmed final amounts";
        mFinalAmountNodesConfirmation[kMessage->senderUUID] = true;
        for (const auto nodeUUIDAndConfirmation : mFinalAmountNodesConfirmation) {
            if (!nodeUUIDAndConfirmation.second) {
                debug() << "Some nodes are still not confirmed final amounts. Waiting.";
                return resultWaitForMessageTypes(
                    {Message::Payments_FinalAmountsConfigurationResponse,
                     Message::Payments_TTLProlongationRequest},
                    maxNetworkDelay(1));
            }
        }
        debug() << "All nodes confirmed final configuration. Begin processing participants votes.";
        return propagateVotesListAndWaitForVoutingResult();
    }
    return reject("Some nodes didn't confirm final amount configuration. Transaction rejected.");
}

PathStats* CoordinatorPaymentTransaction::currentAmountReservationPathStats()
{
    return mPathsStats[mCurrentAmountReservingPathIdentifier].get();
}

void CoordinatorPaymentTransaction::switchToNextPath()
{
    auto justProcessedPath = currentAmountReservationPathStats();
    if (! mPathUUIDs.empty()) {
        mPathUUIDs.erase(mPathUUIDs.cbegin());
    }

    if (mPathUUIDs.size() == 0)
        throw NotFoundError(
            "CoordinatorPaymentTransaction::switchToNextPath: "
            "no paths are available");

    try {
        // to avoid not actual reservations in case of processing path,
        // which contains node on first position, which also is present in path,
        // processed just before, we need delay
        mCurrentAmountReservingPathIdentifier = *mPathUUIDs.cbegin();
        auto currentPath = currentAmountReservationPathStats();
        auto currentFirstIntermediateNode = currentPath->path()->nodes[1];
        auto posCurrentFirstIntermediateNodeInJustProcessedPath = justProcessedPath->path()->positionOfNode(
            currentFirstIntermediateNode);
        if (posCurrentFirstIntermediateNodeInJustProcessedPath > 1
            // if checked node was processed on previous path
            && justProcessedPath->currentIntermediateNodeAndPos().second >
               posCurrentFirstIntermediateNodeInJustProcessedPath) {
            debug() << "delay between process paths to avoid not actual reservations";
            std::this_thread::sleep_for(std::chrono::milliseconds(maxNetworkDelay(1)));
        }
        debug() << "[" << mCurrentAmountReservingPathIdentifier << "] {" << currentPath->path()->toString() << "}";
        // NotFoundError will be always in method justProcessedPath->currentIntermediateNodeAndPos()
        // on this logic it doesn't matter and we ignore it
    } catch (NotFoundError &e) {}
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultOK()
{
    string transactionUUID = mTransactionUUID.stringUUID();
    return transactionResultFromCommand(
        mCommand->responseOK(transactionUUID));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultNoPathsError()
{
    return transactionResultFromCommand(
        mCommand->responseNoRoutes());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultNoResponseError()
{
    return transactionResultFromCommand(
        mCommand->responseRemoteNodeIsInaccessible());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultInsufficientFundsError()
{
    return transactionResultFromCommand(
        mCommand->responseInsufficientFunds());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultNoConsensusError()
{
    return transactionResultFromCommand(
        mCommand->responseNoConsensus());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

void CoordinatorPaymentTransaction::deserializeFromBytes(BytesShared buffer)
{
    BasePaymentTransaction::deserializeFromBytes(buffer);
}

TrustLineAmount CoordinatorPaymentTransaction::totalReservedByAllPaths() const
{
    TrustLineAmount totalAmount = 0;

    for (const auto &pathsStatsKV : mPathsStats) {
        const auto path = pathsStatsKV.second.get();

        if (! path->isValid())
            continue;

        totalAmount += pathsStatsKV.second->maxFlow();
    }

    return totalAmount;
}

const string CoordinatorPaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[CoordinatorPaymentTA: " << currentTransactionUUID() << "] ";
    return s.str();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::approve()
{
    mCommitedAmount = totalReservedAmount(
        AmountReservation::Outgoing);
    BasePaymentTransaction::approve();
    propagateVotesMessageToAllParticipants(
        mParticipantsVotesMessage);
    runBuildThreeNodesCyclesSignal();
    return resultOK();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::reject(
    const char *message)
{
    BasePaymentTransaction::reject(message);

    return resultNoConsensusError();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runDirectAmountReservationResponseProcessingStage ()
{
    debug() << "runDirectAmountReservationResponseProcessingStage";
    auto pathStats = currentAmountReservationPathStats();
    if (not contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        debug() << "No reservation response was received from the receiver node. "
               << "Amount reservation is impossible. Switching to another path.";

        dropReservationsOnPath(
            pathStats,
            mCurrentAmountReservingPathIdentifier);
        mStep = Stages::Coordinator_AmountReservation;
        return tryProcessNextPath();
    }

#ifdef TESTS
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    const auto kMessage = popNextMessage<IntermediateNodeReservationResponseMessage>();

    if (kMessage->state() != IntermediateNodeReservationResponseMessage::Accepted) {
        debug() << "Receiver node rejected reservation. "
               << "Switching to another path.";

        dropReservationsOnPath(
            pathStats,
            mCurrentAmountReservingPathIdentifier);

        mStep = Stages::Coordinator_AmountReservation;
        return tryProcessNextPath();
    }

    pathStats->shortageMaxFlow(kMessage->amountReserved());

    // shortage reservation
    // TODO maybe add if change path->maxFlow()
    auto nodeReservations = mReservations[kMessage->senderUUID];
    for (const auto pathUUIDAndreservation : nodeReservations) {
        if (pathUUIDAndreservation.first == mCurrentAmountReservingPathIdentifier) {
            shortageReservation(
                kMessage->senderUUID,
                pathUUIDAndreservation.second,
                pathStats->maxFlow(),
                mCurrentAmountReservingPathIdentifier);
        }
    }

    const auto kTotalAmount = totalReservedAmount(
        AmountReservation::Outgoing);
    debug() << "Current path reservation finished";
    debug() << "Total collected amount by all paths: " << kTotalAmount;

    if (kTotalAmount > mCommand->amount()){
        error() << "Total requested amount: " << mCommand->amount();
        error() << "Total collected amount is greater than requested amount. "
                << "It indicates that some of the nodes doesn't follows the protocol, "
                << "or that an error is present in protocol itself.";
        rollBack();
        return resultNoConsensusError();
    }

    addFinalConfigurationOnPath(
        mCurrentAmountReservingPathIdentifier,
        pathStats);

    if (kTotalAmount == mCommand->amount()){
        debug() << "Total requested amount: " << mCommand->amount() << ". Collected.";
        debug() << "Begin processing participants votes.";

        // in case of reservation of all transaction amount on direct TL,
        // we skip sendFinalAmountsConfigurationToAllParticipants stage
        return propagateVotesListAndWaitForVoutingResult();
    }
    mStep = Stages::Coordinator_AmountReservation;
    return tryProcessNextPath();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runVotesConsistencyCheckingStage()
{
    debug() << "runVotesConsistencyCheckingStage";
    // Intermediate node or Receiver can send request if transaction is still alive.
    if (contextIsValid(Message::Payments_TTLProlongationRequest, false)) {
        return runTTLTransactionResponce();
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnVoteConsistencyStage(
        mParticipantsVotesMessage->participantsCount());
    mSubsystemsController->testThrowExceptionOnVoteConsistencyStage();
    mSubsystemsController->testTerminateProcessOnVoteConsistencyStage();
#endif

    if (! contextIsValid(Message::Payments_ParticipantsVotes)) {
        return reject("Coordinator didn't receive message with votes");
    }

    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    debug () << "Participants votes message received.";

    mParticipantsVotesMessage = kMessage;
    if (mParticipantsVotesMessage->containsRejectVote()) {
        return reject("Some participant node has been rejected the transaction. Rolling back.");
    }

    if (mParticipantsVotesMessage->achievedConsensus()){
        debug() << "Coordinator received achieved consensus message.";
        if (!checkReservationsDirections()) {
            return reject("Reservations on node are invalid");
        }
        mParticipantsVotesMessage->addParticipant(currentNodeUUID());
        mParticipantsVotesMessage->approve(currentNodeUUID());
        return approve();
    }

    return reject("Coordinator received message with some uncertain votes. Rolling back");
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runTTLTransactionResponce()
{
    debug() << "runTTLTransactionResponce";
    auto kMessage = popNextMessage<TTLProlongationRequestMessage>();
    if (mParticipantsVotesMessage == nullptr) {
        // reservation stage
        if (kMessage->senderUUID == mCommand->contractorUUID()) {
            sendMessage<TTLProlongationResponseMessage>(
                kMessage->senderUUID,
                currentNodeUUID(),
                currentTransactionUUID(),
                TTLProlongationResponseMessage::Continue);
            debug() << "Send clarifying message that transactions is alive to contractor " << kMessage->senderUUID;
        }
        else if (mNodesFinalAmountsConfiguration.find(kMessage->senderUUID) != mNodesFinalAmountsConfiguration.end()) {
            // coordinator has configuration for requested node
            sendMessage<TTLProlongationResponseMessage>(
                kMessage->senderUUID,
                currentNodeUUID(),
                currentTransactionUUID(),
                TTLProlongationResponseMessage::Continue);
            debug() << "Send clarifying message that transactions is alive to node " << kMessage->senderUUID;
        } else {
            sendMessage<TTLProlongationResponseMessage>(
                kMessage->senderUUID,
                currentNodeUUID(),
                currentTransactionUUID(),
                TTLProlongationResponseMessage::Finish);
            debug() << "Send transaction finishing message to node " << kMessage->senderUUID;
        }
    } else {
        // voting stage
        if (mParticipantsVotesMessage->containsParticipant(kMessage->senderUUID)) {
            sendMessage<TTLProlongationResponseMessage>(
                kMessage->senderUUID,
                currentNodeUUID(),
                currentTransactionUUID(),
                TTLProlongationResponseMessage::Continue);
            debug() << "Send clarifying message that transactions is alive to node " << kMessage->senderUUID;
        } else {
            sendMessage<TTLProlongationResponseMessage>(
                kMessage->senderUUID,
                currentNodeUUID(),
                currentTransactionUUID(),
                TTLProlongationResponseMessage::Finish);
            debug() << "Send transaction finishing message to node " << kMessage->senderUUID;
        }
    }
    return resultContinuePreviousState();
}

bool CoordinatorPaymentTransaction::isPathValid(
    Path::Shared path) const
{
    auto itGlobal = path->nodes.begin();
    while (itGlobal != path->nodes.end() - 1) {
        auto itLocal = itGlobal + 1;
        while (itLocal != path->nodes.end()) {
            if (*itGlobal == *itLocal) {
                return false;
            }
            itLocal++;
        }
        itGlobal++;
    }
    return true;
}

void CoordinatorPaymentTransaction::addFinalConfigurationOnPath(
    PathUUID pathUUID,
    PathStats* pathStats)
{
    debug() << "Add final configuration on path " << pathUUID;
    auto pathUUIDAndAmount = make_pair(
        pathUUID,
        make_shared<const TrustLineAmount>(
            pathStats->maxFlow()));

    // add final path configuration for receiver
    if (mNodesFinalAmountsConfiguration.find(mCommand->contractorUUID()) == mNodesFinalAmountsConfiguration.end()) {
        vector<pair<PathUUID, ConstSharedTrustLineAmount>> newVector;
        newVector.push_back(pathUUIDAndAmount);
        mNodesFinalAmountsConfiguration.insert(
            make_pair(
                mCommand->contractorUUID(),
                newVector));
    } else {
        mNodesFinalAmountsConfiguration[mCommand->contractorUUID()].push_back(
            pathUUIDAndAmount);
    }

    // add final path configuration for all intermediate nodes
    for (const auto node : pathStats->path()->intermediateUUIDs()) {
        if (mNodesFinalAmountsConfiguration.find(node) == mNodesFinalAmountsConfiguration.end()) {
            vector<pair<PathUUID, ConstSharedTrustLineAmount>> newVector;
            newVector.push_back(pathUUIDAndAmount);
            mNodesFinalAmountsConfiguration.insert(
                make_pair(
                    node,
                    newVector));
        } else {
            mNodesFinalAmountsConfiguration[node].push_back(
                pathUUIDAndAmount);
        }
    }
}

void CoordinatorPaymentTransaction::savePaymentOperationIntoHistory()
{
    debug() << "savePaymentOperationIntoHistory";
    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->historyStorage()->savePaymentRecord(
        make_shared<PaymentRecord>(
            currentTransactionUUID(),
            PaymentRecord::PaymentOperationType::OutgoingPaymentType,
            mCommand->contractorUUID(),
            mCommitedAmount,
            *mTrustLines->totalBalance().get()));
    debug() << "Operation saved";
}

bool CoordinatorPaymentTransaction::checkReservationsDirections() const
{
    debug() << "checkReservationsDirections";
    for (const auto nodeAndReservations : mReservations) {
        for (const auto pathUUIDAndReservation : nodeAndReservations.second) {
            if (pathUUIDAndReservation.second->direction() != AmountReservation::Outgoing) {
                return false;
            }
        }
    }
    debug() << "All reservations directions are correct";
    return true;
}

void CoordinatorPaymentTransaction::runBuildThreeNodesCyclesSignal()
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
