#include "CycleCloserInitiatorTransaction.h"

CycleCloserInitiatorTransaction::CycleCloserInitiatorTransaction(
    const NodeUUID &kCurrentNodeUUID,
    Path::ConstShared path,
    TrustLinesManager *trustLines,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log)
    noexcept :

    BasePaymentTransaction(
        BaseTransaction::Payments_CycleCloserInitiatorTransaction,
        kCurrentNodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log),
    mCyclesManager(cyclesManager),
    mInitialTransactionAmount(0)
{
    mStep = Stages::Coordinator_Initialisation;
    mPathStats = make_unique<PathStats>(path);
}

CycleCloserInitiatorTransaction::CycleCloserInitiatorTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLines,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log)
    throw (bad_alloc) :

    BasePaymentTransaction(
        buffer,
        nodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log),
    mCyclesManager(cyclesManager)
{}


TransactionResult::SharedConst CycleCloserInitiatorTransaction::run()
    noexcept
{
    try {
        switch (mStep) {
            case Stages::Coordinator_Initialisation:
                return runInitialisationStage();

            case Stages::Coordinator_AmountReservation:
                return runAmountReservationStage();

            case Stages::Coordinator_PreviousNeighborRequestProcessing:
                return runPreviousNeighborRequestProcessingStage();

            case Stages::Common_VotesChecking:
                return runVotesConsistencyCheckingStage();

            case Stages::Common_RollbackByOtherTransaction:
                return runRollbackByOtherTransactionStage();

            default:
                throw RuntimeError(
                    "CycleCloserInitiatorTransaction::run(): "
                       "invalid transaction step.");
        }
    } catch(...) {
        recover("Something happens wrong in method run(). Transaction will be recovered");
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runInitialisationStage()
{
    debug() << "runInitialisationStage";
    // Firstly check if paths is valid cycle
    checkPath(
        mPathStats->path());
    debug() << "cycle is valid";
    const auto kFirstIntermediateNode = mPathStats->path()->nodes[1];
    debug() << "first intermediate node: " << kFirstIntermediateNode;
    if (! mTrustLines->isNeighbor(kFirstIntermediateNode)){
        // Internal process error. Wrong path
        error() << "Invalid path occurred. Node (" << kFirstIntermediateNode << ") is not listed in first level contractors list.";
        throw RuntimeError(
            "CycleCloserInitiatorTransaction::runAmountReservationStage: "
                "invalid first level node occurred. ");
    }
    const auto kOutgoingPossibilities = *(mTrustLines->availableOutgoingCycleAmount(
        kFirstIntermediateNode));
    debug() << "kOutgoingPossibilities: " << kOutgoingPossibilities;

    const auto kLastIntermediateNode = mPathStats->path()->nodes[mPathStats->path()->length() - 2];
    debug() << "last intermediate node: " << kLastIntermediateNode;
    if (! mTrustLines->isNeighbor(kFirstIntermediateNode)){
        // Internal process error. Wrong path
        error() << "Invalid path occurred. Node (" << kLastIntermediateNode << ") is not listed in first level contractors list.";
        throw RuntimeError(
            "CycleCloserInitiatorTransaction::runAmountReservationStage: "
                "invalid first level node occurred. ");
    }
    const auto kIncomingPossibilities = *(mTrustLines->availableIncomingCycleAmount(
        kLastIntermediateNode));
    debug() << "kIncomingPossibilities: " << kIncomingPossibilities;

    mInitialTransactionAmount = min(
        kOutgoingPossibilities,
        kIncomingPossibilities);
    debug() << "mInitialTransactionAmount: " << mInitialTransactionAmount;
    if (mInitialTransactionAmount == 0) {
        debug() << "Can't close cycle, because coordinator incoming or outgoing amount equal zero";
        return resultDone();
    }
    mStep = Stages::Coordinator_AmountReservation;
    return runAmountReservationStage();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runAmountReservationStage ()
{
    debug() << "runAmountReservationStage";
    const auto kPathStats = mPathStats.get();
    if (kPathStats->isReadyToSendNextReservationRequest())
        return tryReserveNextIntermediateNodeAmount(kPathStats);

    else if (kPathStats->isWaitingForNeighborReservationResponse())
        return processNeighborAmountReservationResponse();

    else if (kPathStats->isWaitingForNeighborReservationPropagationResponse())
        return processNeighborFurtherReservationResponse();

    else if (kPathStats->isWaitingForReservationResponse())
        return processRemoteNodeResponse();

    throw RuntimeError(
        "CycleCloserInitiatorTransaction::runAmountReservationStage: "
            "unexpected behaviour occured.");
}

/**
 * @brief CycleCloserInitiatorTransaction::propagateVotesList
 * Collects all nodes from all paths into one votes list,
 * and propagates it to the next node in the votes list.
 */
TransactionResult::SharedConst CycleCloserInitiatorTransaction::propagateVotesListAndWaitForVoutingResult()
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

#ifdef DEBUG
    uint16_t totalParticipantsCount = 0;
#endif

    // If paths wasn't processed - exclude it (all it's nodes).
    // Unprocessed paths may occur, because paths are loaded into the transaction in batch,
    // some of them may be used, and some may be left unprocessed.
    auto kPathStats = mPathStats.get();

    for (const auto &nodeUUID : kPathStats->path()->nodes) {
        // By the protocol, coordinator node must be excluded from the message.
        // Only coordinator may emit ParticipantsApprovingMessage into the network.
        // It is supposed, that in case if it was emitted - than coordinator approved the transaction.
        //
        // TODO: [mvp] [cryptography] despite this, coordinator must sign the message,
        // so the other nodes would be possible to know that this message was emitted by the coordinator.
        if (nodeUUID == kCurrentNodeUUID)
            continue;

        mParticipantsVotesMessage->addParticipant(nodeUUID);

#ifdef DEBUG
        totalParticipantsCount++;
#endif
    }

#ifdef DEBUG
    debug() << "Total participants included: " << totalParticipantsCount;
    debug() << "Participants order is the next:";
    for (const auto kNodeUUIDAndVote : mParticipantsVotesMessage->votes()) {
        debug() << kNodeUUIDAndVote.first;
    }
#endif

    // Begin message propagation
    sendMessage(
        mParticipantsVotesMessage->firstParticipant(),
        mParticipantsVotesMessage);

    debug() << "Votes message constructed and sent to the (" << mParticipantsVotesMessage->firstParticipant() << ")";

    mStep = Stages::Common_VotesChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxNetworkDelay(5));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::tryReserveNextIntermediateNodeAmount (
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
        return resultDone();
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askNeighborToReserveAmount(
    const NodeUUID &neighbor,
    PathStats *path)
{
    debug() << "askNeighborToReserveAmount";
    const auto kCurrentNode = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();

    // Try reserve amount locally.
    path->shortageMaxFlow(mInitialTransactionAmount);
    path->setNodeState(
        1,
        PathStats::NeighbourReservationRequestSent);

    reserveOutgoingAmount(
        neighbor,
        mInitialTransactionAmount,
        0);

    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        neighbor,
        kCurrentNode,
        kTransactionUUID,
        path->maxFlow(),
        path->path()->length());

    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse},
        maxNetworkDelay(1));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askNeighborToApproveFurtherNodeReservation(
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

    sendMessage<CoordinatorCycleReservationRequestMessage>(
        neighbor,
        kCoordinator,
        kTransactionUUID,
        path->maxFlow(),
        kNextAfterNeighborNode);

    debug() << "Further amount reservation request sent to the node (" << neighbor << ") [" << path->maxFlow() << "]";

    path->setNodeState(
        kNeighborPathPosition,
        PathStats::ReservationRequestSent);

    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationResponse},
        kMaxMessageTransferLagMSec);
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::processNeighborAmountReservationResponse()
{
    debug() << "processNeighborAmountReservationResponse";
    if (! contextIsValid(Message::Payments_IntermediateNodeCycleReservationResponse)) {
        debug() << "No neighbor node response received.";
        rollBack();
        return resultDone();
    }

    auto message = popNextMessage<IntermediateNodeCycleReservationResponseMessage>();
    // todo: check message sender

    if (message->state() != IntermediateNodeCycleReservationResponseMessage::Accepted) {
        error() << "Neighbor node doesn't approved reservation request";
        rollBack();
        return resultDone();
    }

    debug() << "(" << message->senderUUID << ") approved reservation request.";
    auto path = mPathStats.get();
    path->setNodeState(
        1, PathStats::NeighbourReservationApproved);
    return runAmountReservationStage();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::processNeighborFurtherReservationResponse()
{
    debug() << "processNeighborFurtherReservationResponse";
    if (! contextIsValid(Message::Payments_CoordinatorCycleReservationResponse)) {
        debug() << "Neighbor node doesn't sent coordinator response.";
        rollBack();
        return resultDone();
    }

    auto message = popNextMessage<CoordinatorCycleReservationResponseMessage>();
    if (message->state() != CoordinatorCycleReservationResponseMessage::Accepted) {
        debug() << "Neighbor node doesn't accepted coordinator request.";
        rollBack();
        return resultDone();
    }

    auto path = mPathStats.get();
    path->setNodeState(
        1,
        PathStats::ReservationApproved);
    debug() << "Neighbor node accepted coordinator request. Reserved: " << message->amountReserved();

    path->shortageMaxFlow(message->amountReserved());
    debug() << "Path max flow is now " << path->maxFlow();

    // shortage reservation
    // TODO maby add if change path->maxFlow()
    for (auto const &itNodeAndReservations : mReservations) {
        auto nodeReservations = itNodeAndReservations.second;
        if (nodeReservations.size() != 1) {
            throw ValueError("CycleCloserInitiatorTransaction::processRemoteNodeResponse: "
                                 "unexpected behaviour: between two nodes should be only one reservation.");
        }
        shortageReservation(
            itNodeAndReservations.first,
            (*nodeReservations.begin()).second,
            path->maxFlow(),
            0);
    }

    if (path->isLastIntermediateNodeProcessed()) {

        const auto kTotalAmount = mPathStats.get()->maxFlow();
        debug() << "Current path reservation finished";
        debug() << "Total collected amount by cycle: " << kTotalAmount;

        return propagateVotesListAndWaitForVoutingResult();
    }

    return runAmountReservationStage();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askRemoteNodeToApproveReservation(
    PathStats* path,
    const NodeUUID& remoteNode,
    const byte remoteNodePosition,
    const NodeUUID& nextNodeAfterRemote)
{
    debug() << "askRemoteNodeToApproveReservation";
    const auto kCoordinator = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();

    sendMessage<CoordinatorCycleReservationRequestMessage>(
        remoteNode,
        kCoordinator,
        kTransactionUUID,
        path->maxFlow(),
        nextNodeAfterRemote);

    path->setNodeState(
        remoteNodePosition,
        PathStats::ReservationRequestSent);

    if (path->isLastIntermediateNodeProcessed()) {
        debug() << "Further amount reservation request sent to the last node (" << remoteNode << ") ["
               << path->maxFlow() << ", next node - (" << nextNodeAfterRemote << ")]";
        mStep = Coordinator_PreviousNeighborRequestProcessing;
        const auto kTimeout = kMaxMessageTransferLagMSec * remoteNodePosition;
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeCycleReservationRequest,
            Message::Payments_CoordinatorCycleReservationResponse},
            kTimeout);
    }
    debug() << "Further amount reservation request sent to the node (" << remoteNode << ") ["
           << path->maxFlow() << ", next node - (" << nextNodeAfterRemote << ")]";

    // Response from te remote node will go throught other nodes in the path.
    // So them would be able to shortage it's reservations (if needed).
    // Total wait timeout must take note of this.
    const auto kTimeout = kMaxMessageTransferLagMSec * remoteNodePosition;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationResponse},
        kTimeout);
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::processRemoteNodeResponse()
{
    debug() << "processRemoteNodeResponse";
    if (! contextIsValid(Message::Payments_CoordinatorCycleReservationResponse)){
        error() << "Can't pay.";
        dropReservationsOnPath(
            mPathStats.get(),
            0);
        return resultDone();
    }

    const auto message = popNextMessage<CoordinatorCycleReservationResponseMessage>();
    auto path = mPathStats.get();

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

        debug() << "Remote node rejected reservation. Can't pay";
        rollBack();
        return resultDone();

    } else {
        const auto reservedAmount = message->amountReserved();

        path->shortageMaxFlow(reservedAmount);
        path->setNodeState(
            R_PathPosition,
            PathStats::ReservationApproved);

        // shortage reservation
        // TODO maby add if change path->maxFlow()
        for (auto const &itNodeAndReservations : mReservations) {
            auto nodeReservations = itNodeAndReservations.second;
            if (nodeReservations.size() != 1) {
                throw ValueError("CycleCloserInitiatorTransaction::processRemoteNodeResponse: "
                                     "unexpected behaviour: between two nodes should be only one reservation.");
            }
            shortageReservation(
                itNodeAndReservations.first,
                (*nodeReservations.begin()).second,
                path->maxFlow(),
                0);
        }

        debug() << "(" << message->senderUUID << ") reserved " << reservedAmount;
        debug() << "Path max flow is now " << path->maxFlow();

        if (path->isLastIntermediateNodeProcessed()) {

            // send final path amount to all intermediate nodes on path
            sendFinalPathConfiguration(
                mPathStats.get(),
                path->maxFlow());
            const auto kTotalAmount = mPathStats.get()->maxFlow();

            debug() << "Current path reservation finished";
            debug() << "Total collected amount by cycle: " << kTotalAmount;

            // this delay is set up to shure that FinalPathConfigurationMessage
            // will be delivered before ParticipantsVotesMessage
            std::this_thread::sleep_for(std::chrono::milliseconds(maxNetworkDelay(1)));
            return propagateVotesListAndWaitForVoutingResult();
        }

        debug() << "Go to the next node in path";
        return tryReserveNextIntermediateNodeAmount(path);
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runPreviousNeighborRequestProcessingStage()
{
    debug() << "runPreviousNeighborRequestProcessingStage";
    if (! contextIsValid(Message::Payments_IntermediateNodeCycleReservationRequest, false))
        return reject("No amount reservation request was received. Rolled back.");

    const auto kMessage = popNextMessage<IntermediateNodeCycleReservationRequestMessage>();
    const auto kNeighbor = kMessage->senderUUID;
    debug() << "Coordiantor payment operation from node (" << kNeighbor << ")";
    debug() << "Requested amount reservation: " << kMessage->amount();

    // Note: (copy of shared pointer is required)
    const auto kIncomingAmount = mTrustLines->availableIncomingCycleAmount(kNeighbor);
    const auto kReservationAmount =
        min(kMessage->amount(), *kIncomingAmount);

    if (0 == kReservationAmount || ! reserveIncomingAmount(kNeighbor, kReservationAmount, 0)) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);
        return reject("No incoming amount reservation is possible. Rolled back.");
    }

    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        kNeighbor,
        currentNodeUUID(),
        currentTransactionUUID(),
        ResponseCycleMessage::Accepted,
        kReservationAmount);

    mStep = Coordinator_AmountReservation;
    const auto kTimeout = kMaxMessageTransferLagMSec;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationResponse},
        kTimeout);
}

void CycleCloserInitiatorTransaction::checkPath(
    const Path::ConstShared path)
{
    if (path->sourceUUID() != path->destinationUUID()) {
        throw ValueError("CycleCloserInitiatorTransaction::checkPath: "
                             "path isn't cycle");
    }
    for (const auto node : path->intermediateUUIDs()) {
        if (node == path->sourceUUID() || node == path->destinationUUID()) {
            throw ValueError("CycleCloserInitiatorTransaction::checkPath: "
                                 "paths contains repeated nodes");
        }
    }
    auto itGlobal = path->nodes.begin() + 1;
    while (itGlobal != path->nodes.end() - 2) {
        auto itLocal = itGlobal + 1;
        while (itLocal != path->nodes.end() - 1) {
            if (*itGlobal == *itLocal) {
                throw ValueError("CycleCloserInitiatorTransaction::checkPath: "
                                     "paths contains repeated nodes");
            }
            itLocal++;
        }
        itGlobal++;
    }
}

void CycleCloserInitiatorTransaction::sendFinalPathConfiguration(
    PathStats* pathStats,
    const TrustLineAmount &finalPathAmount)
{
    debug() << "sendFinalPathConfiguration";
    for (const auto &intermediateNode : pathStats->path()->intermediateUUIDs()) {
        debug() << "send message with final path amount info for node " << intermediateNode;
        sendMessage<FinalPathCycleConfigurationMessage>(
            intermediateNode,
            currentNodeUUID(),
            currentTransactionUUID(),
            finalPathAmount);
    }
}

const NodeUUID& CycleCloserInitiatorTransaction::coordinatorUUID() const
{
    return currentNodeUUID();
}

const uint8_t CycleCloserInitiatorTransaction::cycleLength() const
{
    return (uint8_t)mPathStats->path()->length();
}

pair<BytesShared, size_t> CycleCloserInitiatorTransaction::serializeToBytes() const
    throw (bad_alloc)
{
    // todo: add implementation
    return make_pair(make_shared<byte>(0), 0);
}

void CycleCloserInitiatorTransaction::deserializeFromBytes(BytesShared buffer)
{
    // todo: add implementation
}

const string CycleCloserInitiatorTransaction::logHeader() const
{
    stringstream s;
    s << "[CycleCloserInitiatorTA: " << currentTransactionUUID() << "] ";
    return s.str();
}