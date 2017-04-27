#include "CoordinatorPaymentTransaction.h"

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    const NodeUUID &kCurrentNodeUUID,
    const CreditUsageCommand::Shared kCommand,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    ResourcesManager *resourcesManager,
    Logger *log)
    noexcept :

    BasePaymentTransaction(
        BaseTransaction::CoordinatorPaymentTransaction,
        kCurrentNodeUUID,
        trustLines,
        storageHandler,
        log),
    mCommand(kCommand),
    mResourcesManager(resourcesManager),
    mReservationsStage(0),
    mDirectPathIsAllreadyProcessed(false)
{
    mStep = Stages::Coordinator_Initialisation;
}

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Logger *log)
    throw (bad_alloc) :
    BasePaymentTransaction(
        BaseTransaction::CoordinatorPaymentTransaction,
        buffer,
        trustLines,
        storageHandler,
        log)
{}


TransactionResult::SharedConst CoordinatorPaymentTransaction::run()
throw (RuntimeError, bad_alloc)
{
    cout << "CoordinatorPaymentTransaction"  << to_string(mStep) << endl;
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

        case Stages::Coordinator_FinalPathsConfigurationApproving:
            return runFinalParticipantsRequestsProcessingStage();

        case Stages::Common_VotesChecking:
            return runVotesCheckingStage();

        case Stages::Common_VotesRecoveryStage:
            return runVotesRecoveryParenStage();

        default:
            throw RuntimeError(
                "CoordinatorPaymentTransaction::run(): "
                    "invalid transaction step.");
    }
}


TransactionResult::SharedConst CoordinatorPaymentTransaction::runPaymentInitialisationStage ()
{
    info() << "Operation initialised to the node (" << mCommand->contractorUUID() << ")";
    info() << "Command UUID: " << mCommand->UUID();
    info() << "Operation amount: " << mCommand->amount();


    if (mCommand->contractorUUID() == currentNodeUUID()) {
        info() << "Attempt to initialise operation against itself was prevented. Canceled.";
        return resultProtocolError();
    }


    // Check if total outgoing possibilities of this node are not smaller,
    // than total operation amount. In case if so - there is no reason to begin the operation:
    // current node would not be able to pay such an amount.
    const auto kTotalOutgoingPossibilities = *(mTrustLines->totalOutgoingAmount());
    if (kTotalOutgoingPossibilities < mCommand->amount())
        return resultInsufficientFundsError();


    // TODO: Read paths from paths manager.
    // TODO: Ensure paths shuffling

    NodeUUID sender = currentNodeUUID();
//    NodeUUID b("13e5cf8c-5834-4e52-b65b-f9281dd1ff01");
//    NodeUUID c("13e5cf8c-5834-4e52-b65b-f9281dd1ff02");
//    NodeUUID receiver("13e5cf8c-5834-4e52-b65b-f9281dd1ff03");
    //    auto p1 = make_shared<const Path>(
//        Path(sender, receiver));
//    auto p2 = make_shared<const Path>(
//        Path(sender, receiver, {c}));

//    addPathForFurtherProcessing(p1);
//    addPathForFurtherProcessing(p2);

// TODO for test
    NodeUUID *nodeUUID51Ptr = new NodeUUID("f6b1d2ab-19b9-4085-b943-a362ad011865");
    NodeUUID *nodeUUID52Ptr = new NodeUUID("a718dd56-7704-4fb5-94ef-3dbbc4195b2f");
    NodeUUID *nodeUUID53Ptr = new NodeUUID("c3642755-7b0a-4420-b7b0-2dcf578d88ca");
    NodeUUID *nodeUUID54Ptr = new NodeUUID("a2da327a-8d38-4466-bf8c-c5b3eac0025e");
    NodeUUID *nodeUUID55Ptr = new NodeUUID("f1eccb61-9c0e-4ff7-9d5e-a40726fb6901");

    vector<NodeUUID> intermediateNodes;
    intermediateNodes.push_back(*nodeUUID52Ptr);
    intermediateNodes.push_back(*nodeUUID53Ptr);
    intermediateNodes.push_back(*nodeUUID54Ptr);
    auto result = make_shared<const Path>(
        *nodeUUID51Ptr,
        *nodeUUID55Ptr,
        intermediateNodes);

    delete nodeUUID51Ptr;
    delete nodeUUID52Ptr;
    delete nodeUUID53Ptr;
    delete nodeUUID54Ptr;
    delete nodeUUID55Ptr;

    addPathForFurtherProcessing(result);
    // end test

    mResourcesManager->requestPaths(
        currentTransactionUUID(),
        mCommand->contractorUUID());

    mStep = Stages::Coordinator_ReceiverResourceProcessing;
    return transactionResultFromState(
        TransactionState::waitForResourcesTypes(
            {BaseResource::ResourceType::Paths},
            kMaxResourceTransferLagMSec));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runReceiverResourceProcessingStage()
{
    if (mResources.size() == 1) {
        auto responseResource = *mResources.begin();
        if (responseResource->type() == BaseResource::ResourceType::Paths) {
            PathsResource::Shared response = static_pointer_cast<PathsResource>(
                responseResource);

            // TODO for test
            NodeUUID *nodeUUID51Ptr = new NodeUUID("f6b1d2ab-19b9-4085-b943-a362ad011865");
            NodeUUID *nodeUUID52Ptr = new NodeUUID("a718dd56-7704-4fb5-94ef-3dbbc4195b2f");
            NodeUUID *nodeUUID53Ptr = new NodeUUID("c3642755-7b0a-4420-b7b0-2dcf578d88ca");
            NodeUUID *nodeUUID54Ptr = new NodeUUID("a2da327a-8d38-4466-bf8c-c5b3eac0025e");
            NodeUUID *nodeUUID55Ptr = new NodeUUID("f1eccb61-9c0e-4ff7-9d5e-a40726fb6901");

            vector<NodeUUID> intermediateNodes;
            intermediateNodes.push_back(*nodeUUID52Ptr);
            intermediateNodes.push_back(*nodeUUID53Ptr);
            intermediateNodes.push_back(*nodeUUID54Ptr);
            auto result = make_shared<const Path>(
                *nodeUUID51Ptr,
                *nodeUUID55Ptr,
                intermediateNodes);

            delete nodeUUID51Ptr;
            delete nodeUUID52Ptr;
            delete nodeUUID53Ptr;
            delete nodeUUID54Ptr;
            delete nodeUUID55Ptr;

            addPathForFurtherProcessing(result);
            // end test

//            response->pathCollection()->resetCurrentPath();
//            while (response->pathCollection()->hasNextPath()) {
//                auto path = response->pathCollection()->nextPath();
//                addPathForFurtherProcessing(path);
//            }
        } else {
            // TODO: action on this case
            error() << "wrong resource type";
        }
    } else {
        // TODO: action on this case
        error() << "wrong resources size";
    }



    // If there is no one path to the receiver - transaction can't proceed.
    if (mPathsStats.empty())
        return resultNoPathsError();

    info() << "Collected paths:";
    for (const auto &identifierAndStats : mPathsStats)
        info() << "{" << identifierAndStats.second->path()->toString() << "}";


    // Sending message to the receiver note to approve the payment receiving.
    sendMessage<ReceiverInitPaymentRequestMessage>(
        mCommand->contractorUUID(),
        currentNodeUUID(),
        currentTransactionUUID(),
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
    if (kMessage->state() != ReceiverInitPaymentResponseMessage::Accepted)
        return exitWithResult(
            resultDone(),
            "Receiver rejected payment operation. Canceling.");


    info() << "Receiver accepted operation. Begin reserving amounts.";
    mStep = Stages::Coordinator_AmountReservation;
    return resultFlushAndContinue();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runAmountReservationStage ()
{
    switch (mReservationsStage) {
    case 0: {
            initAmountsReservationOnNextPath();
            mReservationsStage += 1;
            cout << "mReservationsStage: " << to_string(mReservationsStage) << endl;

            // Note:
            // next section must be executed immediately.
            // (no "break" is needed).
        }

    case 1: {
            const auto kPathStats = currentAmountReservationPathStats();
            if (kPathStats->path()->length() == 2) {
                // In case if path contains only sender and receiver -
                // middleware nodes reservation must be omitted.
                return tryReserveAmountDirectlyOnReceiver(kPathStats);
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
                "unexpected behaviour occured.");
        }

    default:
        throw ValueError(
            "CoordinatorPaymentTransaction::processAmountReservationStage: "
            "unexpected reservations stage occured.");
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runFinalParticipantsRequestsProcessingStage ()
{
    if (! contextIsValid(Message::Payments_ParticipantsPathsConfigurationRequest))
        // Coordinator already signed the transaction and can't reject it.
        // But the remote intermediate node will newer receive
        // the response and must not sign the transaction.
        return recover("No final configuration request was received. Recovering.");


    const auto kMessage = popNextMessage<ParticipantsConfigurationRequestMessage>();
    info() << "Final payment paths configuration request received from (" << kMessage->senderUUID << ")";


    if (kMessage->senderUUID == mCommand->contractorUUID()){
        // Receiver requested final payment configuration.
        auto responseMessage = make_shared<ParticipantsConfigurationMessage>(
            currentNodeUUID(),
            currentTransactionUUID(),
            ParticipantsConfigurationMessage::ForReceiverNode);

        for (const auto &pathUUIDAndPathStats : mPathsStats) {
            const auto kPathStats = pathUUIDAndPathStats.second.get();
            const auto kPath = kPathStats->path();

            if (pathUUIDAndPathStats.second->path()->length() > 2)
                // If paths wasn't processed - exclude it.
                if (!kPathStats->isLastIntermediateNodeProcessed())
                    continue;

            if (pathUUIDAndPathStats.second->path()->length() > 2)
                // If path was dropped (processed, but rejected) - exclude it.
                if (!kPathStats->isValid())
                    continue;

            const auto kReceiverPathPos = kPath->length();
            const auto kIncomingNode = kPath->nodes[kReceiverPathPos - 1];

            responseMessage->addPath(
                kPathStats->maxFlow(),
                kIncomingNode);

#ifdef DEBUG
            debug() << "Added path: ("
                    << kIncomingNode << ") ["
                    << kPathStats->maxFlow() << "]";
#endif
        }

        const auto kReceiverNodeUUID = kMessage->senderUUID;
        sendMessage(
            kReceiverNodeUUID,
            responseMessage);

#ifdef DEBUG
        debug() << "Final payment path configuration message sent to the (" << kReceiverNodeUUID << ")";
#endif

    } else {
        // Intermediate node requested final payment configuration.
        auto responseMessage = make_shared<ParticipantsConfigurationMessage>(
            currentNodeUUID(),
            currentTransactionUUID(),
            ParticipantsConfigurationMessage::ForIntermediateNode);

        for (const auto &pathUUIDAndPathStats : mPathsStats) {
            const auto kPathStats = pathUUIDAndPathStats.second.get();
            const auto kPath = kPathStats->path();

            // If paths wasn't processed - exclude it.
            if (!kPathStats->isLastIntermediateNodeProcessed())
                continue;

            // If path was dropped (processed, but rejected) - exclude it.
            if (!kPathStats->isValid())
                continue;


            auto kIntermediateNodePathPos = 1;
            while (kIntermediateNodePathPos != kPath->length()) {
                if (kPath->nodes[kIntermediateNodePathPos] == kMessage->senderUUID)
                    break;

                kIntermediateNodePathPos++;
            }

            const auto kIncomingNode = kPath->nodes[kIntermediateNodePathPos - 1];
            const auto kOutgoingNode = kPath->nodes[kIntermediateNodePathPos + 1];

            responseMessage->addPath(
                kPathStats->maxFlow(),
                kIncomingNode,
                kOutgoingNode);

#ifdef DEBUG
            debug() << "Added path: ("
                    << kIncomingNode << "), ("
                    << kOutgoingNode << ") ["
                    << kPathStats->maxFlow() << "]";
#endif
        }

        const auto kReceiverNodeUUID = kMessage->senderUUID;
        sendMessage(
            kReceiverNodeUUID,
            responseMessage);

#ifdef DEBUG
        debug() << "Final payment path configuration message sent to the (" << kReceiverNodeUUID << ")";
#endif
    }


    mNodesRequestedFinalConfiguration.insert(kMessage->senderUUID);
    if (mNodesRequestedFinalConfiguration.size() == mParticipantsVotesMessage->participantsCount()){

#ifdef DEBUG
        debug() << "All involved nodes has been requested final payment path configuration. "
                   "Begin waiting for the signed votes message";
#endif

        mStep = Stages::Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(2));
    }

    // Waiting for the rest nodes to request final payment paths configuration
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsPathsConfigurationRequest},
        maxNetworkDelay(1));
}


TransactionResult::SharedConst CoordinatorPaymentTransaction::runVotesCheckingStage ()
{
    // Votes message may be received twice:
    // First time - as a request to check the transaction and to sing it in case if all correct.
    // Second time - as a command to commit/rollback the transaction.
    if (mTransactionIsVoted)
        return runVotesConsistencyCheckingStage();


    if (! contextIsValid(Message::Payments_ParticipantsVotes))
        return reject("No participants votes received. Canceling.");


    const auto kCurrentNodeUUID = currentNodeUUID();
    auto message = popNextMessage<ParticipantsVotesMessage>();
    info() << "Votes message received";


    try {
        // Check if current node is listed in the votes list.
        // This check is needed to prevent processing message in case of missdelivering.
        message->vote(kCurrentNodeUUID);

    } catch (NotFoundError &) {
        // It seems that current node wasn't listed in the votes list.
        // This is possible only in case, when one node takes part in 2 parallel transactions,
        // that have common UUID (transactions UUIDs collision).
        // The probability of this is very small, but is present.
        //
        // In this case - the message must be simply ignored.

        info() << "Votes message ignored due to transactions UUIDs collision detected.";
        info() << "Waiting for another votes message.";

        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(message->participantsCount())); // ToDo: kMessage->participantsCount() must not be used (it is invalid)
    }


    if (message->containsRejectVote())
        // Some node rejected the transaction.
        // This node must simply roll back it's part of transaction and exit.
        // No further message propagation is needed.
        reject("Some participant node has been rejected the transaction. Rolling back.");


    // Votes message must be saved for further processing on next awakening.
    mParticipantsVotesMessage = message;


    mStep = Stages::Common_FinalPathsConfigurationChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsPathsConfiguration},
        maxCoordinatorResponseTimeout());
}


/**
 * @brief CoordinatorPaymentTransaction::propagateVotesList
 * Collects all nodes from all paths into one votes list,
 * and propagates it to the next node in the votes list.
 */
TransactionResult::SharedConst CoordinatorPaymentTransaction::propagateVotesListAndWaitForConfigurationRequests ()
{
    const auto kCurrentNodeUUID = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();

    // TODO: additional check if payment is correct

    // Prevent simple transaction rolling back
    // todo: make this atomic
    mTransactionIsVoted = true;

    auto message = make_shared<ParticipantsVotesMessage>(
        kCurrentNodeUUID,
        kTransactionUUID,
        kCurrentNodeUUID);

#ifdef DEBUG
    uint16_t totalParticipantsCount = 0;
#endif

    for (const auto &pathUUIDAndPathStats : mPathsStats) {
        // If paths wasn't processed - exclude it (all it's nodes).
        // Unprocessed paths may occur, because paths are loaded into the transaction in batch,
        // some of them may be used, and some may be left unprocessed.
        if (pathUUIDAndPathStats.second->path()->length() > 2)
            if (! pathUUIDAndPathStats.second->isLastIntermediateNodeProcessed())
                continue;

        // If path was dropped - exclude it (all it's nodes).
        // Paths may be dropped in case if some node doesn't approved reservation,
        // or, in case if there is no free amount on it.
        if (pathUUIDAndPathStats.second->path()->length() > 2)
            if (! pathUUIDAndPathStats.second->isValid())
                continue;


        for (const auto &nodeUUID : pathUUIDAndPathStats.second->path()->nodes) {
            // By the protocol, coordinator node must be excluded from the message.
            // Only coordinator may emit ParticipantsApprovingMessage into the network.
            // It is supposed, that in case if it was emitted - than coordinator approved the transaction.
            //
            // TODO: [mvp] [cryptography] despite this, coordinator must sign the message,
            // so the other nodes would be possible to know that this message was emitted by the coordinator.
            if (nodeUUID == kCurrentNodeUUID)
                continue;

            message->addParticipant(nodeUUID);

#ifdef DEBUG
            totalParticipantsCount++;
#endif
        }
    }


#ifdef DEBUG
    debug() << "Total participants included: " << totalParticipantsCount;
    debug() << "Participants order is the next:";
    for (const auto kNodeUUIDAndVote : message->votes()) {
        debug() << kNodeUUIDAndVote.first;
    }
#endif

    // Begin message propagation
    sendMessage(
        message->firstParticipant(),
        message);

    info() << "Votes message constructed and sent to the (" << message->firstParticipant() << ")";
    info() << "Begin accepting final payment paths configuration requests.";


    // Participants votes message would be used further
    // in final paths configurations requests processing stage.
    mParticipantsVotesMessage = message;

    // Now coordinator begins responding to the final configuration requests of the participants.
    mStep = Stages::Coordinator_FinalPathsConfigurationApproving;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsPathsConfigurationRequest},
        maxNetworkDelay(1));
}


void CoordinatorPaymentTransaction::initAmountsReservationOnNextPath()
{
    if (mPathsStats.empty())
        throw NotFoundError(
            "CoordinatorPaymentTransaction::tryBlockAmounts: "
            "no paths are available.");

    mCurrentAmountReservingPathIdentifier = mPathsStats.begin()->first;
}

/*
 * Tries to reserve amount on path that consists only of sender and receiver nodes.
 */
TransactionResult::SharedConst CoordinatorPaymentTransaction::tryReserveAmountDirectlyOnReceiver (
    PathStats *pathStats)
{
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


    info() << "Direct path occurred (coordinator -> receiver). "
           << "Trying to reserve amount directly on the receiver side.";


    const auto kCoordinator = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();
    const auto kContractor = mCommand->contractorUUID();


    // ToDo: implement operator < for TrustLineAmount and remove this pure conversion

    // Check if local reservation is possible.
    // If not - there is no reason to send any reservations requests.
    const auto kAvailableOutgoingAmount = mTrustLines->availableOutgoingAmount(kContractor);
    if (*kAvailableOutgoingAmount == TrustLineAmount(0)) {
        info() << "There is no direct outgoing amount available for the receiver node. "
               << "Switching to another path.";

        pathStats->setUnusable();
        return tryProcessNextPath();
    }

    // Reserving amount locally.
    const auto kReservationAmount = min(mCommand->amount(), *kAvailableOutgoingAmount);
    if (not reserveOutgoingAmount(kContractor, kReservationAmount)){
        error() << "Can't reserve amount locally. "
                << "Switching to another path.";

        pathStats->setUnusable();
        return tryProcessNextPath();
    }

    // Reserving on the contractor side
    pathStats->shortageMaxFlow(kReservationAmount);
    sendMessage<IntermediateNodeReservationRequestMessage>(
        kContractor,
        kCoordinator,
        kTransactionUUID,
        kReservationAmount);

    info() << "Reservation request for " << *kAvailableOutgoingAmount << " sent directly to the receiver node.";


    mStep = Stages::Coordinator_ShortPathAmountReservationResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse},
        maxNetworkDelay(1));
}


TransactionResult::SharedConst CoordinatorPaymentTransaction::tryReserveNextIntermediateNodeAmount (
    PathStats *pathStats)
{
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
            info() << "Processing " << int(R_PathPosition) << " node in path: (" << R_UUID << ").";

            return askRemoteNodeToApproveReservation(
                pathStats,
                R_UUID,
                R_PathPosition,
                S_UUID);
        }

    } catch(NotFoundError) {
        info() << "No unprocessed paths are left.";
        info() << "Requested amount can't be collected. Canceling.";
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

    for (;;) {
        // Cycle is needed to prevent possible hashes collision.
        PathUUID identifier = boost::uuids::random_generator()();
        if (mPathsStats.count(identifier) == 0){
            mPathsStats[identifier] = make_unique<PathStats>(path);
            return;
        }
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askNeighborToReserveAmount(
    const NodeUUID &neighbor,
    PathStats *path)
{
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
    // TODO : резервувати mCommand->amount() - емаунт уже зарезервованих траст ліній
    const auto kReservationAmount = min(*kAvailableOutgoingAmount, mCommand->amount());

    if (kReservationAmount == 0) {
        info() << "No payment amount is available for (" << neighbor << "). "
                  "Switching to another path.";

        path->setUnusable();
        return tryProcessNextPath();
    }

    // Try reserve amount locally.
    path->shortageMaxFlow(kReservationAmount);
    path->setNodeState(
        1,
        PathStats::NeighbourReservationRequestSent);

    reserveOutgoingAmount(
        neighbor,
        kReservationAmount);


    sendMessage<IntermediateNodeReservationRequestMessage>(
        neighbor,
        kCurrentNode,
        kTransactionUUID,
        path->maxFlow());

    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse},
        maxNetworkDelay(1));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askNeighborToApproveFurtherNodeReservation(
    const NodeUUID& neighbor,
    PathStats *path)
{
    const auto kCoordinator = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();
    const auto kNeighborPathPosition = 1;
    const auto kNextAfterNeighborNode = path->path()->nodes[kNeighborPathPosition+1];

    // Note:
    // no check of "neighbor" node is needed here.
    // It was done on previous step.


    sendMessage<CoordinatorReservationRequestMessage>(
        neighbor,
        kCoordinator,
        kTransactionUUID,
        path->maxFlow(),
        kNextAfterNeighborNode);

    info() << "Further amount reservation request sent to the node (" << neighbor << ") [" << path->maxFlow() << "]";

    path->setNodeState(
        kNeighborPathPosition,
        PathStats::ReservationRequestSent);


    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationResponse},
        kMaxMessageTransferLagMSec);
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processNeighborAmountReservationResponse()
{
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        info() << "No neighbor node response received. Switching to another path.";
        return tryProcessNextPath();
    }


    auto message = popNextMessage<IntermediateNodeReservationResponseMessage>();
    // todo: check message sender

    if (message->state() != IntermediateNodeReservationResponseMessage::Accepted) {
        error() << "Neighbor node doesn't approved reservation request";
        return tryProcessNextPath();
    }


    info() << "(" << message->senderUUID << ") approved reservation request.";
    auto path = currentAmountReservationPathStats();
    path->setNodeState(
        1, PathStats::NeighbourReservationApproved);


    return runAmountReservationStage();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processNeighborFurtherReservationResponse()
{
    if (! contextIsValid(Message::Payments_CoordinatorReservationResponse)) {
        info() << "Switching to another path.";
        return tryProcessNextPath();
    }

    auto message = popNextMessage<CoordinatorReservationResponseMessage>();
    if (message->state() != CoordinatorReservationResponseMessage::Accepted) {
        info() << "Neighbor node doesn't accepted coordinator request.";
        return tryProcessNextPath();
    }


    auto path = currentAmountReservationPathStats();
    path->setNodeState(
        1,
        PathStats::ReservationApproved);
    info() << "Neighbor node accepted coordinator request. Reserved: " << message->amountReserved();

    path->shortageMaxFlow(message->amountReserved());
    info() << "Path max flow is now " << path->maxFlow();


    return runAmountReservationStage();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askRemoteNodeToApproveReservation(
    PathStats* path,
    const NodeUUID& remoteNode,
    const byte remoteNodePosition,
    const NodeUUID& nextNodeAfterRemote)
{
    const auto kCoordinator = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();

    sendMessage<CoordinatorReservationRequestMessage>(
        remoteNode,
        kCoordinator,
        kTransactionUUID,
        path->maxFlow(),
        nextNodeAfterRemote);

    path->setNodeState(
        remoteNodePosition,
        PathStats::ReservationRequestSent);

    info() << "Further amount reservation request sent to the node (" << remoteNode << ") ["
           << path->maxFlow() << ", next node - (" << nextNodeAfterRemote << ")]";

    // Response from te remote node will go throught other nodes in the path.
    // So them would be able to shortage it's reservations (if needed).
    // Total wait timeout must take note of this.
    const auto kTimeout = kMaxMessageTransferLagMSec * remoteNodePosition;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationResponse},
        kTimeout);
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processRemoteNodeResponse()
{
    if (! contextIsValid(Message::Payments_CoordinatorReservationResponse)){
        info() << "Switching to another path.";
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


    if (0 == message->amountReserved()) {
        path->setUnusable();
        path->setNodeState(
            R_PathPosition,
            PathStats::ReservationRejected);

        info() << "Remote node rejected reservation. Switching to another path.";
        return tryProcessNextPath();

    } else {
        const auto reservedAmount = message->amountReserved();

        path->shortageMaxFlow(reservedAmount);
        path->setNodeState(
            R_PathPosition,
            PathStats::ReservationApproved);

        info() << "(" << message->senderUUID << ") reserved " << reservedAmount;
        info() << "Path max flow is now " << path->maxFlow();

        if (path->isLastIntermediateNodeProcessed()) {
            const auto kTotalAmount = totalReservedByAllPaths();

            info() << "Current path reservation finished";
            info() << "Total collected amount by all paths: " << kTotalAmount;

            if (kTotalAmount > mCommand->amount()){
                error() << "Total requested amount: " << mCommand->amount();
                error() << "Total collected amount is greater than requested amount. "
                           "It indicates that some of the nodes doesn't follows the protocol, "
                           "or that an error is present in protocol itself.";

                return resultDone();
            }

            if (kTotalAmount == mCommand->amount()){
                info() << "Total requested amount: " << mCommand->amount() << ". Collected.";
                info() << "Begin processing participants votes.";

                return propagateVotesListAndWaitForConfigurationRequests();
            }
        }

        info() << "Switching to another path";
        return tryReserveNextIntermediateNodeAmount(path);
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::tryProcessNextPath()
{
    try {
        switchToNextPath();
        return runAmountReservationStage();

    } catch (Exception &e) {
        info() << "No another paths are available. Canceling.";
        return resultInsufficientFundsError();
    }
}

PathStats* CoordinatorPaymentTransaction::currentAmountReservationPathStats()
{
    return mPathsStats[mCurrentAmountReservingPathIdentifier].get();
}

void CoordinatorPaymentTransaction::switchToNextPath()
{
    if (! mPathsStats.empty())
        mPathsStats.erase(mPathsStats.cbegin());

    if (mPathsStats.size() == 0)
        throw NotFoundError(
            "CoordinatorPaymentTransaction::switchToNextPath: "
            "no paths are available");

    mCurrentAmountReservingPathIdentifier = mPathsStats.begin()->first;
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->responseOK());
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

pair<BytesShared, size_t> CoordinatorPaymentTransaction::serializeToBytes() const
    throw (bad_alloc)
{
    // todo: add implementation
    return make_pair(make_shared<byte>(0), 0);
}

void CoordinatorPaymentTransaction::deserializeFromBytes(BytesShared buffer)
{
    // todo: add implementation
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
    s << "[CoordinatorPaymentTA: " << currentTransactionUUID().stringUUID() << "] ";

    return s.str();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::approve()
{
    BasePaymentTransaction::approve();
    return resultOK();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::recover(
    const char *message)
{
    BasePaymentTransaction::recover(
        message);

    // TODO: implement me correct.
    return resultProtocolError();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::reject(
    const char *message)
{
    BasePaymentTransaction::reject(message);

    return resultNoConsensusError();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runDirectAmountReservationResponseProcessingStage ()
{
    if (not contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        info() << "No reservation response was received from the receiver node. "
               << "Amount reservation is impossible. Switching to another path.";

        mStep = Stages::Coordinator_AmountReservation;
        return tryProcessNextPath();
    }


    auto pathStats = currentAmountReservationPathStats();
    const auto kMessage = popNextMessage<IntermediateNodeReservationResponseMessage>();

    if (not kMessage->state() == IntermediateNodeReservationResponseMessage::Accepted) {
        info() << "Receiver node rejected reservation. "
               << "Switching to another path.";

        pathStats->setUnusable();

        mStep = Stages::Coordinator_AmountReservation;
        return tryProcessNextPath();
    }


    const auto kTotalAmount = totalReservedByAllPaths();
    info() << "Current path reservation finished";
    info() << "Total collected amount by all paths: " << kTotalAmount;

    if (kTotalAmount > mCommand->amount()){
        error() << "Total requested amount: " << mCommand->amount();
        error() << "Total collected amount is greater than requested amount. "
                << "It indicates that some of the nodes doesn't follows the protocol, "
                << "or that an error is present in protocol itself.";

        return resultDone();
    }

    if (kTotalAmount == mCommand->amount()){
        info() << "Total requested amount: " << mCommand->amount() << ". Collected.";
        info() << "Begin processing participants votes.";

        return propagateVotesListAndWaitForConfigurationRequests();
    }
}
