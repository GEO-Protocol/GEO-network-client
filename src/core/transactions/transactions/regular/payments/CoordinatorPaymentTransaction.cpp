#include "CoordinatorPaymentTransaction.h"


CoordinatorPaymentTransaction::PathStats::PathStats(
    Path::ConstShared path) :

    mPath(path),
    mIntermediateNodesStates(
        path->length()-2, // intermediate nodes count
        ReservationRequestDoesntSent),
    mMaxPathFlow(0),
    mIsValid(true)
{}

void CoordinatorPaymentTransaction::PathStats::setNodeState(
    const uint8_t positionInPath,
    const CoordinatorPaymentTransaction::PathStats::NodeState state)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(positionInPath > 0);
    assert(positionInPath <= mIntermediateNodesStates.size());
#endif

    // TODO: add check of state degradation

    // Note: only intermediate nodes are present in mNodesStates,
    // so the -1 is required.
    mIntermediateNodesStates[positionInPath-1] = state;
}

const TrustLineAmount&CoordinatorPaymentTransaction::PathStats::maxFlow() const
{
    return mMaxPathFlow;
}

void CoordinatorPaymentTransaction::PathStats::setMaxFlow(
    const TrustLineAmount& amount)
{
    if (mMaxPathFlow == 0) {
        mMaxPathFlow = amount;

    } else if (amount < mMaxPathFlow) {
        // next node reservation shortened current flow;
        mMaxPathFlow = amount;
    }
}

const Path::ConstShared CoordinatorPaymentTransaction::PathStats::path() const
{
    return mPath;
}

const pair<NodeUUID, uint8_t> CoordinatorPaymentTransaction::PathStats::currentIntermediateNodeAndPos() const
{
    for (uint8_t i=0; i<mIntermediateNodesStates.size(); ++i) {
        if (mIntermediateNodesStates[i] != PathStats::ReservationApproved &&
            mIntermediateNodesStates[i] != PathStats::ReservationRejected)
            return make_pair(mPath->nodes[i+1], i+1);
    }

    throw NotFoundError(
        "CoordinatorPaymentTransaction::PathStats::currentNodeRequestMustBeSent: "
        "no unprocessed nodes are left.");
}

/**
 * @returns node to which amount reservation request wasn't set yet.
 * Amount reservation request may be both:
 * "CoordinatorReservationRequest" and "IntermediateNodeReservationRequest".
 * (this method ensures both requests are sent to the node)
 *
 * Also, node position (relative to the source node) would be returned.
 *
 * @throws NotFoundError - in case if all nodes of this path are already processed.
 */
const pair<NodeUUID, uint8_t> CoordinatorPaymentTransaction::PathStats::nextIntermediateNodeAndPos() const
{
    for (uint8_t i=0; i<mIntermediateNodesStates.size(); ++i) {
        if (0==i &&
            mIntermediateNodesStates[i] == PathStats::NeighbourReservationApproved) {
            return make_pair(mPath->nodes[i+1], i+1);
        }

        if (mIntermediateNodesStates[i] == PathStats::ReservationRequestDoesntSent) {
            return make_pair(mPath->nodes[i+1], i+1);
        }
    }

    throw NotFoundError(
        "CoordinatorPaymentTransaction::PathStats::nextNodeRequestMustBeSent: "
        "no unprocessed nodes are left.");
}

const bool CoordinatorPaymentTransaction::PathStats::reservationRequestSentToAllNodes() const
{
    return
        mIntermediateNodesStates.at(
            mIntermediateNodesStates.size()-1) != ReservationRequestDoesntSent;
}

const bool CoordinatorPaymentTransaction::PathStats::isNeighborAmountReserved() const
{
    return mIntermediateNodesStates[0] == PathStats::NeighbourReservationApproved;
}

const bool CoordinatorPaymentTransaction::PathStats::isWaitingForNeighborReservationResponse() const
{
    return mIntermediateNodesStates[0] == PathStats::NeighbourReservationRequestSent;
}

const bool CoordinatorPaymentTransaction::PathStats::isWaitingForNeighborReservationPropagationResponse() const
{
    return mIntermediateNodesStates[0] == PathStats::ReservationRequestSent;
}

/**
 * @returns true if current path sent amount reservation request and
 * is now waiting for the response to it.
 */
const bool CoordinatorPaymentTransaction::PathStats::isWaitingForReservationResponse() const
{
    for (const auto& it: mIntermediateNodesStates) {
        if (it == PathStats::ReservationRequestSent)
            return true;
    }

    return false;
}

const bool CoordinatorPaymentTransaction::PathStats::isReadyToSendNextReservationRequest() const
{
    return !isWaitingForReservationResponse() &&
           !isWaitingForNeighborReservationResponse() &&
           !isLastIntermediateNodeProcessed();
}

const bool CoordinatorPaymentTransaction::PathStats::isLastIntermediateNodeProcessed() const
{
    return
        mIntermediateNodesStates[mIntermediateNodesStates.size()-1] !=
            PathStats::ReservationRequestDoesntSent;
}

const bool CoordinatorPaymentTransaction::PathStats::isValid() const
{
    return mIsValid;
}

void CoordinatorPaymentTransaction::PathStats::setUnusable()
{
    mIsValid = false;
    mMaxPathFlow = 0;
}


CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    const NodeUUID &currentNodeUUID,
    CreditUsageCommand::Shared command,
    TrustLinesManager *trustLines,
    Logger *log) :

    BasePaymentTransaction(
        BaseTransaction::CoordinatorPaymentTransaction,
        currentNodeUUID,
        trustLines,
        log),
    mCommand(command),
    mReservationsStage(0)
{}

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log):

    BasePaymentTransaction(
        BaseTransaction::CoordinatorPaymentTransaction,
        buffer,
        trustLines,
        log)
{}


TransactionResult::SharedConst CoordinatorPaymentTransaction::run()
{
    switch (mStep) {
    case Stages::ReceiverPaymentInitialisation:
        return initPaymentOperation();

    case Stages::ReceiverResponseProcessing:
        return processReceiverResponse();

    case Stages::AmountReservation:
        return tryReserveAmounts();

    default:
        throw RuntimeError(
                    "CoordinatorPaymentTransaction::run(): "
                    "invalid transaction step.");
    }
}


TransactionResult::SharedConst CoordinatorPaymentTransaction::initPaymentOperation()
{
    info() << "Operation initialised to the node (" << mCommand->contractorUUID() << ")";
    info() << "Command UUID: " << mCommand->UUID();
    info() << "Operation amount: " << mCommand->amount();


    if (mCommand->contractorUUID() == nodeUUID()) {
        // TODO: cycle operation?
        info() << "Attempt to initialise operation against itself was prevented. Canceled.";
        return resultProtocolError();
    }

    // TODO: optimisation
    // Check if total outgoing possibilities of this node
    // are not smaller, than total operation amount.
    // In case if so - there is no reason to begin the operation:
    // current node would not be able to pay such an amount.


    // TODO: Read paths from paths manager.
    // TODO: Ensure paths shuffling
    // TODO: optimisation: if no paths are available - no operation can be proceed.

    NodeUUID sender = nodeUUID();
    NodeUUID b("13e5cf8c-5834-4e52-b65b-f9281dd1ff01");
    NodeUUID c("13e5cf8c-5834-4e52-b65b-f9281dd1ff02");
    NodeUUID receiver("13e5cf8c-5834-4e52-b65b-f9281dd1ff03");

    auto p1 = make_shared<const Path>(
        Path(sender, receiver, {b, c}));
//    auto p2 = make_shared<const Path>(
//        Path(sender, receiver, {c}));

    addPathForFurtherProcessing(p1);
//    addPathForFurtherProcessing(p2);


    // If there is no one path to the receiver -
    // transaction can't proceed.
    //
    // TODO: load more paths from paths manager.
    if (mPathsStats.empty()) {
        return resultNoPathsError();
    }

    info() << "Collected paths:";
    for (const auto &identifierAndStats : mPathsStats) {
        info() << "{" << identifierAndStats.second->path()->toString() << "}";
    }


    sendMessage<ReceiverInitPaymentRequestMessage>(
        mCommand->contractorUUID(),
        nodeUUID(),
        UUID(),
        mCommand->amount());

    mStep = Stages::ReceiverResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_ReceiverInitPaymentResponse},
        kMaxMessageTransferLagMSec);
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processReceiverResponse()
{
    if (! validateContext(Message::Payments_ReceiverInitPaymentResponse)){
        info() << "Can't proceed. Canceling.";
        return resultNoResponseError();
    }


    const auto kMessage = popNextMessage<ReceiverInitPaymentResponseMessage>();
    if (! kMessage->state() == ReceiverInitPaymentResponseMessage::Accepted) {
        info() << "Receiver rejected payment operation. Canceling.";
        return resultExit();
    }

    info() << "Receiver accepted operation. Moving to amounts reservation stage.";

    mStep = Stages::AmountReservation;
    return resultFlushAndContinue();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::tryReserveAmounts()
{
    switch (mReservationsStage) {
    case 0: {
            initAmountsReservationOnNextPath();
            mReservationsStage += 1;

            // Note:
            // next section must be executed immediately.
            // (no "break" is needed).
        }

    case 1: {
            auto path = currentAmountReservationPathStats();
            if (path->isReadyToSendNextReservationRequest())
                return tryReserveNextNodeAmount(path);

            else if (path->isWaitingForNeighborReservationResponse())
                return processNeighborAmountReservationResponse();

            else if (path->isWaitingForNeighborReservationPropagationResponse())
                return processNeighborFurtherReservationResponse();

            else if (path->isWaitingForReservationResponse())
                return processRemoteNodeResponse();

            throw RuntimeError(
                "CoordinatorPaymentTransaction::tryReserveAmounts: "
                "unexpected behaviour occured.");
        }

    default:
        throw ValueError(
            "CoordinatorPaymentTransaction::tryReserveAmounts: "
            "unexpected reservations stage occured.");
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processApprovesCollectingStage()
{
    mStep = Stages::ApprovesCollecting;
    info() << "Approves collecting stage started.";


    const auto kCurrentNodeUUID = nodeUUID();
    const auto kTransactionUUID = UUID();

    auto message = make_shared<ParticipantsApprovingMessage>(
        kCurrentNodeUUID,
        kTransactionUUID);

    size_t totalParticipantsCount = 0;
    for (const auto &pathUUIDAndPathStats : mPathsStats) {
        if (! pathUUIDAndPathStats.second->isLastIntermediateNodeProcessed())
            continue;

        if (! pathUUIDAndPathStats.second->isValid())
            continue;

        for (const auto &nodeUUID : pathUUIDAndPathStats.second->path()->nodes) {
            // By the protocol,
            // coordinator node must be excluded from the message.
            //
            // Only coordinator may emit ParticipantsApprovingMessage into the network.
            // It is supposed, that in case if it was emitted - than coordinator approved the transaction.
            //
            // TODO: [mvp] [cryptograpy] despite this, coordinator must sign the message,
            // so the other nodes would be possible to know that this message was emitted by the coordinator.
            if (nodeUUID == kCurrentNodeUUID)
                continue;

            message->addParticipant(nodeUUID);
            totalParticipantsCount++;


            info() << "Node (" << nodeUUID << ") included as participant";
        }
    }
    info() << "Total participants included: " << totalParticipantsCount;


    // Sending the message to the first node in votes list.
    sendMessage(
        message->firstParticipant(),
        message);


    // Now coordinator node must wait for this message
    // with all participants approved, or one participant rejected.
    const auto kTotalProcessingTimeout =
        (totalParticipantsCount * kMaxMessageTransferLagMSec)
        + (totalParticipantsCount * 3000); // delay for message processing
    info() << "Max response waiting timeout for this operation " << kTotalProcessingTimeout;


    // Move to the next stage
    mStep = Stages::ApprovesChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsApprove},
        kTotalProcessingTimeout);
}

void CoordinatorPaymentTransaction::initAmountsReservationOnNextPath()
{
    if (mPathsStats.empty())
        throw NotFoundError(
            "CoordinatorPaymentTransaction::tryBlockAmounts: "
            "no paths are available.");

    mCurrentAmountReservingPathIdentifier = mPathsStats.begin()->first;
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::tryReserveNextNodeAmount(
    PathStats* pathStats)
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
        // Cylce is needed to prevent possible hashes collison.
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
    const auto currentNode = nodeUUID();
    const auto transactionUUID = UUID();


    info() << "Processing neighbor amount reservation";

    if (! mTrustLines->isNeighbor(neighbor)){
        // Internal process error.
        // No next path must be selected.
        // Transaction execution must be cancelled.

        error() << "Invalid path occured. Node (" << neighbor << ") is not listed in first level contractors list.";
        error() << "This may signal about protocol/data manipulations.";

        throw RuntimeError(
            "CoordinatorPaymentTransaction::tryReserveNextNodeAmount: "
            "invalid first level node occured. ");
    }

    const auto reservationAmount =  mTrustLines->availableOutgoingAmount(neighbor);
    if (*reservationAmount == 0) {
        info() << "No payment amount is available for (" << neighbor << "). "
                  "Switching to another path.";

        path->setUnusable();
        return tryProcessNextPath();
    }

    info() << "Amount reserved locally: " << *reservationAmount;

    path->setMaxFlow(*reservationAmount);
    info() << "Path max flow updated to " << path->maxFlow();


    sendMessage<IntermediateNodeReservationRequestMessage>(
        neighbor,
        currentNode,
        transactionUUID,
        path->maxFlow());

    path->setNodeState(
        1, PathStats::NeighbourReservationRequestSent);


    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse},
        kMaxMessageTransferLagMSec);
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askNeighborToApproveFurtherNodeReservation(
    const NodeUUID& neighbor,
    PathStats *path)
{
    const auto kCoordinator = nodeUUID();
    const auto kTransactionUUID = UUID();
    const auto kNeighborPathPosition = 1;
    const auto kNextAfterNeighborNode = path->path()->nodes[kNeighborPathPosition+1];

    // Note:
    // no check of "neighbor" node is not needed here.
    // It was done on previous step.

    sendMessage<CoordinatorReservationRequestMessage>(
        neighbor,
        kCoordinator,
        kTransactionUUID,
        path->maxFlow(),
        kNextAfterNeighborNode);


    path->setNodeState(
        kNeighborPathPosition,
        PathStats::ReservationRequestSent);


    info() << "Coordinator request sent to the neighbor node";
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationResponse},
        kMaxMessageTransferLagMSec);
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processNeighborAmountReservationResponse()
{
    if (! validateContext(Message::Payments_IntermediateNodeReservationResponse)) {
        info() << "Switching to another path.";
        return tryProcessNextPath();
    }


    auto message = popNextMessage<IntermediateNodeReservationResponseMessage>();
    if (message->state() != IntermediateNodeReservationResponseMessage::Accepted) {
        error() << "Neighbor node doesn't approved reservation request";
        error() << "It seems that trust lines on both sides are inconsistent.";

        // TODO: run consystency establishing transaction
        return tryProcessNextPath();
    }


    info() << "Neighbor node succesfully approved reservation";
    auto path = currentAmountReservationPathStats();
    path->setNodeState(
        1, PathStats::NeighbourReservationApproved);


    return tryReserveAmounts();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processNeighborFurtherReservationResponse()
{
    if (! validateContext(Message::Payments_CoordinatorReservationResponse)) {
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

    path->setMaxFlow(message->amountReserved());
    info() << "Path max flow is now " << path->maxFlow();


    return tryReserveAmounts();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askRemoteNodeToApproveReservation(
    PathStats* path,
    const NodeUUID& remoteNode,
    const byte remoteNodePosition,
    const NodeUUID& nextNodeAfterRemote)
{
    const auto kCoordinator = nodeUUID();
    const auto kTransactionUUID = UUID();

    sendMessage<CoordinatorReservationRequestMessage>(
        remoteNode,
        kCoordinator,
        kTransactionUUID,
        path->maxFlow(),
        nextNodeAfterRemote);

    path->setNodeState(
        remoteNodePosition,
        PathStats::ReservationRequestSent);

    info() << "Amount reservation request sent to the node (" << remoteNode << ")";
    info() << "Requested reservation amount is: " << path->maxFlow();
    info() << "Next path node is: (" << nextNodeAfterRemote << ")";

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
    if (! validateContext(Message::Payments_CoordinatorReservationResponse)){
        info() << "Switching to another path.";
        return tryProcessNextPath();
    }


    const auto message = popNextMessage<CoordinatorReservationResponseMessage>();
    auto path = currentAmountReservationPathStats();

    /*
     * Nodes scheme:
     *  R - remote node;
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

        path->setMaxFlow(reservedAmount);
        path->setNodeState(
            R_PathPosition,
            PathStats::ReservationApproved);

        info() << "Remote node approved reservation of " << reservedAmount;
        info() << "Path max flow is now " << path->maxFlow();

        if (path->isLastIntermediateNodeProcessed()) {
            const auto kTotalAmount = totalReservedByAllPaths();

            info() << "Current path reservation ended";
            info() << "Total collected amount: " << kTotalAmount;

            if (kTotalAmount > mCommand->amount()){
                error() << "Total collected amount by all paths: " << kTotalAmount;
                error() << "Total requested amount: " << mCommand->amount();
                error() << "Total collected amount is greater than requested amount. "
                           "It indicates that some of the nodes doesn't follows the protocol, "
                           "or that an error is present in protocol itself.";

                return resultExit();
            }

            if (kTotalAmount == mCommand->amount()){
                info() << "Requested amount collected";
                info() << "Moving to approves collecting stage";

                return processApprovesCollectingStage();
            }
        }

        info() << "Switching to another path";
        return tryReserveNextNodeAmount(path);
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::tryProcessNextPath()
{
    try {
        switchToNextPath();
        return tryReserveAmounts();

    } catch (Exception &e) {
        info() << "No another paths are available. Canceling.";
        return resultInsufficientFundsError();
    }
}

CoordinatorPaymentTransaction::PathStats* CoordinatorPaymentTransaction::currentAmountReservationPathStats()
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

pair<BytesShared, size_t> CoordinatorPaymentTransaction::serializeToBytes() const
{
    throw ValueError("Not implemented");
}

void CoordinatorPaymentTransaction::deserializeFromBytes(BytesShared buffer)
{
    throw ValueError("Not implemented");
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
    s << "[CoordinatorPaymentTA: " << UUID().stringUUID() << "] ";

    return s.str();
}
