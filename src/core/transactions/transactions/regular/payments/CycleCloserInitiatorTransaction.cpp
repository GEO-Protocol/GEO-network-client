#include "CycleCloserInitiatorTransaction.h"

CycleCloserInitiatorTransaction::CycleCloserInitiatorTransaction(
    const NodeUUID &kCurrentNodeUUID,
    Path::ConstShared path,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Logger *log)
    noexcept :

    BasePaymentTransaction(
        BaseTransaction::Payments_CycleCloserInitiatorTransaction,
        kCurrentNodeUUID,
        trustLines,
        storageHandler,
        log),
    mInitialTransactionAmount(0)
{
    mStep = Stages::Coordinator_Initialisation;
    mPathStats = make_unique<PathStats>(path);
}

CycleCloserInitiatorTransaction::CycleCloserInitiatorTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Logger *log)
    throw (bad_alloc) :

    BasePaymentTransaction(
        buffer,
        nodeUUID,
        trustLines,
        storageHandler,
        log)
{}


TransactionResult::SharedConst CycleCloserInitiatorTransaction::run()
    throw (RuntimeError, bad_alloc)
{
    switch (mStep) {
        case Stages::Coordinator_Initialisation:
            return runInitialisationStage();

        case Stages::Coordinator_AmountReservation:
            return runAmountReservationStage();

        case Stages::Coordinator_PreviousNeighborRequestProcessing:
            return runPreviousNeighborRequestProcessingStage();

        case Stages::Coordinator_FinalPathsConfigurationApproving:
            return runFinalParticipantsRequestsProcessingStage();

        case Stages::Common_VotesChecking:
            return runVotesConsistencyCheckingStage();

        default:
            throw RuntimeError(
                "CycleCloserInitiatorTransaction::run(): "
                    "invalid transaction step.");
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runInitialisationStage()
{
    // Firstly check if paths is valid cycle
    checkPath(
        mPathStats->path());

    const auto kFirstIntermediateNode = mPathStats->path()->nodes[1];
    if (! mTrustLines->isNeighbor(kFirstIntermediateNode)){
        // Internal process error. Wrong path
        error() << "Invalid path occurred. Node (" << kFirstIntermediateNode << ") is not listed in first level contractors list.";
        throw RuntimeError(
            "CycleCloserInitiatorTransaction::runAmountReservationStage: "
                "invalid first level node occurred. ");
    }
    const auto kOutgoingPossibilities = *(mTrustLines->availableOutgoingAmount(
        kFirstIntermediateNode));

    const auto kLastIntermediateNode = mPathStats->path()->nodes[mPathStats->path()->length() - 2];
    if (! mTrustLines->isNeighbor(kFirstIntermediateNode)){
        // Internal process error. Wrong path
        error() << "Invalid path occurred. Node (" << kLastIntermediateNode << ") is not listed in first level contractors list.";
        throw RuntimeError(
            "CycleCloserInitiatorTransaction::runAmountReservationStage: "
                "invalid first level node occurred. ");
    }
    const auto kIncomingPossibilities = *(mTrustLines->availableIncomingAmount(
        kLastIntermediateNode));

    mInitialTransactionAmount = min(
        kOutgoingPossibilities,
        kIncomingPossibilities);
    if (mInitialTransactionAmount == 0) {
        info() << "Can't close cycle, because coordinator incoming or outgoing amount equal zero";
        return resultDone();
    }
    mStep = Stages::Coordinator_AmountReservation;
    return resultFlushAndContinue();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runAmountReservationStage ()
{
    info() << "runAmountReservationStage";
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

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runFinalParticipantsRequestsProcessingStage ()
{
    info() << "runFinalParticipantsRequestsProcessingStage";
    if (! contextIsValid(Message::Payments_ParticipantsPathsConfigurationRequest))
        // Coordinator already signed the transaction and can't reject it.
        // But the remote intermediate node will newer receive
        // the response and must not sign the transaction.
        return recover("No final configuration request was received. Recovering.");

    const auto kMessage = popNextMessage<ParticipantsConfigurationRequestMessage>();
    info() << "Final payment paths configuration request received from (" << kMessage->senderUUID << ")";

    // Intermediate node requested final payment configuration.
    auto responseMessage = make_shared<ParticipantsConfigurationMessage>(
        currentNodeUUID(),
        currentTransactionUUID(),
        ParticipantsConfigurationMessage::ForIntermediateNode);

    const auto kPathStats = mPathStats.get();
    const auto kPath = kPathStats->path();

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
        kOutgoingNode,
        0);

#ifdef DEBUG
    debug() << "Added path: ("
            << kIncomingNode << "), ("
            << kOutgoingNode << ") ["
            << kPathStats->maxFlow() << "]";
#endif

    const auto kReceiverNodeUUID = kMessage->senderUUID;
    sendMessage(
        kReceiverNodeUUID,
        responseMessage);

#ifdef DEBUG
    debug() << "Final payment path configuration message sent to the (" << kReceiverNodeUUID << ")";
#endif

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

/**
 * @brief CycleCloserInitiatorTransaction::propagateVotesList
 * Collects all nodes from all paths into one votes list,
 * and propagates it to the next node in the votes list.
 */
TransactionResult::SharedConst CycleCloserInitiatorTransaction::propagateVotesListAndWaitForConfigurationRequests ()
{
    info() << "propagateVotesListAndWaitForConfigurationRequests";
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

        message->addParticipant(nodeUUID);

#ifdef DEBUG
        totalParticipantsCount++;
#endif
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

TransactionResult::SharedConst CycleCloserInitiatorTransaction::tryReserveNextIntermediateNodeAmount (
    PathStats *pathStats)
{
    info() << "propagateVotesListAndWaitForConfigurationRequests";
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
        return resultDone();
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askNeighborToReserveAmount(
    const NodeUUID &neighbor,
    PathStats *path)
{
    info() << "askNeighborToReserveAmount";
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

    sendMessage<IntermediateNodeReservationRequestMessage>(
        neighbor,
        kCurrentNode,
        kTransactionUUID,
        0,
        path->maxFlow());

    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse},
        maxNetworkDelay(1));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askNeighborToApproveFurtherNodeReservation(
    const NodeUUID& neighbor,
    PathStats *path)
{
    info() << "askNeighborToApproveFurtherNodeReservation";
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
        0,
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

TransactionResult::SharedConst CycleCloserInitiatorTransaction::processNeighborAmountReservationResponse()
{
    info() << "processNeighborAmountReservationResponse";
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        info() << "No neighbor node response received.";
        rollBack();
        return resultDone();
    }

    auto message = popNextMessage<IntermediateNodeReservationResponseMessage>();
    // todo: check message sender

    if (message->state() != IntermediateNodeReservationResponseMessage::Accepted) {
        error() << "Neighbor node doesn't approved reservation request";
        rollBack();
        return resultDone();
    }

    info() << "(" << message->senderUUID << ") approved reservation request.";
    auto path = mPathStats.get();
    path->setNodeState(
        1, PathStats::NeighbourReservationApproved);
    return runAmountReservationStage();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::processNeighborFurtherReservationResponse()
{
    info() << "processNeighborFurtherReservationResponse";
    if (! contextIsValid(Message::Payments_CoordinatorReservationResponse)) {
        info() << "Neighbor node doesn't sent coordinator response.";
        rollBack();
        return resultDone();
    }

    auto message = popNextMessage<CoordinatorReservationResponseMessage>();
    if (message->state() != CoordinatorReservationResponseMessage::Accepted) {
        info() << "Neighbor node doesn't accepted coordinator request.";
        rollBack();
        return resultDone();
    }

    auto path = mPathStats.get();
    path->setNodeState(
        1,
        PathStats::ReservationApproved);
    info() << "Neighbor node accepted coordinator request. Reserved: " << message->amountReserved();

    path->shortageMaxFlow(message->amountReserved());
    info() << "Path max flow is now " << path->maxFlow();

    // shortage reservation
    // TODO maby add if change path->maxFlow()
    auto localReservationsCopy = mReservations;
    for (auto const &itNodeAndReservations : localReservationsCopy) {
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

    return runAmountReservationStage();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askRemoteNodeToApproveReservation(
    PathStats* path,
    const NodeUUID& remoteNode,
    const byte remoteNodePosition,
    const NodeUUID& nextNodeAfterRemote)
{
    info() << "askRemoteNodeToApproveReservation";
    const auto kCoordinator = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();

    sendMessage<CoordinatorReservationRequestMessage>(
        remoteNode,
        kCoordinator,
        kTransactionUUID,
        0,
        path->maxFlow(),
        nextNodeAfterRemote);

    path->setNodeState(
        remoteNodePosition,
        PathStats::ReservationRequestSent);

    if (path->isLastIntermediateNodeProcessed()) {
        info() << "Further amount reservation request sent to the last node (" << remoteNode << ") ["
               << path->maxFlow() << ", next node - (" << nextNodeAfterRemote << ")]";
        mStep = Coordinator_PreviousNeighborRequestProcessing;
        const auto kTimeout = kMaxMessageTransferLagMSec * remoteNodePosition;
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
            Message::Payments_CoordinatorReservationResponse},
            kTimeout);
    }
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

TransactionResult::SharedConst CycleCloserInitiatorTransaction::processRemoteNodeResponse()
{
    info() << "processRemoteNodeResponse";
    if (! contextIsValid(Message::Payments_CoordinatorReservationResponse)){
        error() << "Can't pay.";
        rollBack();
        return resultDone();
    }

    const auto message = popNextMessage<CoordinatorReservationResponseMessage>();
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

        info() << "Remote node rejected reservation. Can't pay";
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
        auto localReservationsCopy = mReservations;
        for (auto const &itNodeAndReservations : localReservationsCopy) {
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

        info() << "(" << message->senderUUID << ") reserved " << reservedAmount;
        info() << "Path max flow is now " << path->maxFlow();

        if (path->isLastIntermediateNodeProcessed()) {
            const auto kTotalAmount = mPathStats.get()->maxFlow();

            info() << "Current path reservation finished";
            info() << "Total collected amount by cycle: " << kTotalAmount;

            return propagateVotesListAndWaitForConfigurationRequests();
        }

        info() << "Go to the next node in path";
        return tryReserveNextIntermediateNodeAmount(path);
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runPreviousNeighborRequestProcessingStage()
{
    info() << "runPreviousNeighborRequestProcessingStage";
    // TODO : add other checking or change error() message in contextIsValid()
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationRequest))
        return reject("No amount reservation request was received. Rolled back.");

    const auto kMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();
    const auto kNeighbor = kMessage->senderUUID;
    info() << "Coordiantor payment operation from node (" << kNeighbor << ")";
    info() << "Requested amount reservation: " << kMessage->amount();

    // Note: (copy of shared pointer is required)
    const auto kIncomingAmount = mTrustLines->availableIncomingAmount(kNeighbor);
    const auto kReservationAmount =
        min(kMessage->amount(), *kIncomingAmount);

    if (0 == kReservationAmount || ! reserveIncomingAmount(kNeighbor, kReservationAmount, 0)) {
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            0,
            ResponseMessage::Rejected);
        return reject("No incoming amount reservation is possible. Rolled back.");
    }

    sendMessage<IntermediateNodeReservationResponseMessage>(
        kNeighbor,
        currentNodeUUID(),
        currentTransactionUUID(),
        0,
        ResponseMessage::Accepted,
        kReservationAmount);

    mStep = Coordinator_AmountReservation;
    const auto kTimeout = kMaxMessageTransferLagMSec;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationResponse},
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
    auto itGlobal = path->intermediateUUIDs().begin();
    while (itGlobal != path->intermediateUUIDs().end() - 1) {
        auto itLocal = itGlobal + 1;
        while (itLocal != path->intermediateUUIDs().end()) {
            if (*itGlobal == *itLocal) {
                throw ValueError("CycleCloserInitiatorTransaction::checkPath: "
                                     "paths contains repeated nodes");
            }
            itLocal++;
        }
        itGlobal++;
    }
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
    s << "[CycleCloserInitiatorTA: " << currentTransactionUUID().stringUUID() << "] ";
    return s.str();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::reject(
    const char *message)
{
    BasePaymentTransaction::reject(message);
    return resultDone();
}