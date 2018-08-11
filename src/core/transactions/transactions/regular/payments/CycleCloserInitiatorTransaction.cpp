#include "CycleCloserInitiatorTransaction.h"

CycleCloserInitiatorTransaction::CycleCloserInitiatorTransaction(
    const NodeUUID &kCurrentNodeUUID,
    Path::ConstShared path,
    const SerializedEquivalent equivalent,
    TrustLinesManager *trustLines,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController)
noexcept :

    BasePaymentTransaction(
        BaseTransaction::Payments_CycleCloserInitiatorTransaction,
        kCurrentNodeUUID,
        equivalent,
        trustLines,
        storageHandler,
        topologyCacheManager,
        maxFlowCacheManager,
        keystore,
        log,
        subsystemsController),
    mCyclesManager(cyclesManager)
{
    mStep = Stages::Coordinator_Initialization;
    mPathStats = make_unique<PathStats>(path);
}

CycleCloserInitiatorTransaction::CycleCloserInitiatorTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLines,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController)
throw (bad_alloc) :

    BasePaymentTransaction(
        buffer,
        nodeUUID,
        trustLines,
        storageHandler,
        topologyCacheManager,
        maxFlowCacheManager,
        keystore,
        log,
        subsystemsController),
    mCyclesManager(cyclesManager)
{}


TransactionResult::SharedConst CycleCloserInitiatorTransaction::run()
    noexcept
{
    try {
        switch (mStep) {
            case Stages::Coordinator_Initialization:
                return runInitializationStage();

            case Stages::Coordinator_AmountReservation:
                return runAmountReservationStage();

            case Stages::Coordinator_PreviousNeighborRequestProcessing:
                return runPreviousNeighborRequestProcessingStage();

            case Stages::Coordinator_FinalAmountsConfigurationConfirmation:
                return runFinalAmountsConfigurationConfirmationProcessingStage();

            case Stages::Common_VotesChecking:
                return runVotesConsistencyCheckingStage();

            case Stages::Cycles_WaitForIncomingAmountReleasing:
                return runPreviousNeighborRequestProcessingStageAgain();

            case Stages::Cycles_WaitForOutgoingAmountReleasing:
                return runAmountReservationStageAgain();

            case Stages::Common_RollbackByOtherTransaction:
                return runRollbackByOtherTransactionStage();

            default:
                throw RuntimeError(
                    "CycleCloserInitiatorTransaction::run(): "
                       "invalid transaction step.");
        }
    } catch(Exception &e) {
        warning() << e.what();
        return reject("Something happens wrong in method run(). Transaction will be rejected");
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runInitializationStage()
{
    debug() << "runInitializationStage";
    // Firstly check if paths is valid cycle
    checkPath(
        mPathStats->path());
    debug() << "cycle is valid";
    mNextNode = mPathStats->path()->nodes[1];
    debug() << "first intermediate node: " << mNextNode;
    if (! mTrustLines->trustLineIsPresent(mNextNode)){
        // Internal process error. Wrong path
        warning() << "Invalid path occurred. Node (" << mNextNode << ") is not listed in first level contractors list.";
        throw RuntimeError(
            "CycleCloserInitiatorTransaction::runAmountReservationStage: "
                "invalid first level node occurred. ");
    }

    const auto kOutgoingAmounts = mTrustLines->availableOutgoingCycleAmounts(mNextNode);
    const auto kOutgoingAmountWithReservations = kOutgoingAmounts.first;
    const auto kOutgoingAmountWithoutReservations = kOutgoingAmounts.second;
    debug() << "OutgoingAmountWithReservations: " << *kOutgoingAmountWithReservations
            << " OutgoingAmountWithoutReservations: " << *kOutgoingAmountWithoutReservations;

    if (*kOutgoingAmountWithReservations == TrustLine::kZeroAmount()) {
        if (*kOutgoingAmountWithoutReservations == TrustLine::kZeroAmount()) {
            warning() << "Can't close cycle, because coordinator outgoing amount equal zero, "
                "and can't use reservations from other transactions";
            mCyclesManager->addClosedTrustLine(
                currentNodeUUID(),
                mNextNode);
            return resultDone();
        } else {
            mOutgoingAmount = TrustLineAmount(0);
        }
    } else {
        mOutgoingAmount = *kOutgoingAmountWithReservations;
    }

    debug() << "outgoing Possibilities: " << mOutgoingAmount;

    mPreviousNode = mPathStats->path()->nodes[mPathStats->path()->length() - 2];
    debug() << "last intermediate node: " << mPreviousNode;
    if (! mTrustLines->trustLineIsPresent(mPreviousNode)){
        // Internal process error. Wrong path
        warning() << "Invalid path occurred. Node (" << mPreviousNode << ") is not listed in first level contractors list.";
        throw RuntimeError(
            "CycleCloserInitiatorTransaction::runAmountReservationStage: "
                "invalid first level node occurred. ");
    }

    const auto kIncomingAmounts = mTrustLines->availableIncomingCycleAmounts(mPreviousNode);
    mIncomingAmount = *(kIncomingAmounts.first);

    if (mIncomingAmount == TrustLine::kZeroAmount()) {
        warning() << "Can't close cycle, because coordinator incoming amount equal zero";
        mCyclesManager->addClosedTrustLine(
            mPreviousNode,
            currentNodeUUID());
        return resultDone();
    }
    debug() << "Incoming Possibilities: " << mIncomingAmount;

    if (mIncomingAmount < mOutgoingAmount) {
        mOutgoingAmount = mIncomingAmount;
    }
    debug() << "Initial Outgoing Amount: " << mOutgoingAmount;
    mStep = Stages::Coordinator_AmountReservation;
    return runAmountReservationStage();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runAmountReservationStage ()
{
    debug() << "runAmountReservationStage";
    if (mPathStats->isReadyToSendNextReservationRequest())
        return tryReserveNextIntermediateNodeAmount();

    else if (mPathStats->isWaitingForNeighborReservationResponse())
        return processNeighborAmountReservationResponse();

    else if (mPathStats->isWaitingForNeighborReservationPropagationResponse())
        return processNeighborFurtherReservationResponse();

    else if (mPathStats->isWaitingForReservationResponse())
        return processRemoteNodeResponse();

    throw RuntimeError(
        "CycleCloserInitiatorTransaction::runAmountReservationStage: "
            "unexpected behaviour occurred.");
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::propagateVotesListAndWaitForVotingResult()
{
    debug() << "propagateVotesListAndWaitForVotingResult";

    // TODO: additional check if payment is correct

    mParticipantsSignatures.clear();

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToNextNodeOnVoteStage();
#endif

    auto ioTransaction = mStorageHandler->beginTransaction();
    mPublicKey = mKeysStore->generateAndSaveKeyPairForPaymentTransaction(
        ioTransaction,
        currentTransactionUUID(),
        mNodeUUID);
    mParticipantsPublicKeys.insert(
        make_pair(
            mPaymentNodesIds[mNodeUUID],
            mPublicKey));

    // send message with all public keys to all participants and wait for voting results
    for (const auto &nodeAndPaymentID : mPaymentNodesIds) {
        if (nodeAndPaymentID.first == mNodeUUID) {
            continue;
        }
        sendMessage<ParticipantsPublicKeysMessage>(
            nodeAndPaymentID.first,
            mEquivalent,
            mNodeUUID,
            currentTransactionUUID(),
            mParticipantsPublicKeys);
    }

#ifdef TESTS
    mSubsystemsController->testThrowExceptionOnVoteStage();
    mSubsystemsController->testTerminateProcessOnVoteStage();
#endif

    mStep = Stages::Common_VotesChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantVote},
        maxNetworkDelay(6));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::tryReserveNextIntermediateNodeAmount ()
{
    debug() << "tryReserveNextIntermediateNodeAmount";
    /*
     * Nodes scheme:
     *  R - remote node;
     *  S - next node in path after remote one;
     */

    try {
        const auto R_UUIDAndPos = mPathStats->nextIntermediateNodeAndPos();
        const auto R_UUID = R_UUIDAndPos.first;
        const auto R_PathPosition = R_UUIDAndPos.second;

        const auto S_PathPosition = R_PathPosition + 1;
        const auto S_UUID = mPathStats->path()->nodes[S_PathPosition];

        if (R_PathPosition == 1) {
            if (mPathStats->isNeighborAmountReserved())
                return askNeighborToApproveFurtherNodeReservation();

            else
                return askNeighborToReserveAmount();

        } else {
            debug() << "Processing " << int(R_PathPosition) << " node in path: (" << R_UUID << ").";

            return askRemoteNodeToApproveReservation(
                R_UUID,
                R_PathPosition,
                S_UUID);
        }

    } catch(NotFoundError) {
        warning() << "No unprocessed paths are left. Requested amount can't be collected. Canceling.";
        return resultDone();
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askNeighborToReserveAmount()
{
    debug() << "askNeighborToReserveAmount";
    const auto kCurrentNode = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();

    // Try reserve amount locally.
    mPathStats->shortageMaxFlow(mOutgoingAmount);
    mPathStats->setNodeState(
        1,
        PathStats::NeighbourReservationRequestSent);

    if (0 == mOutgoingAmount) {
        // try use reservations from other transactions
        auto reservations = mTrustLines->reservationsToContractor(mNextNode);
        for (auto &reservation : reservations) {
            debug() << "try use " << reservation->amount() << " from "
                    << reservation->transactionUUID() << " transaction";
            if (mCyclesManager->resolveReservationConflict(
                currentTransactionUUID(), reservation->transactionUUID())) {
                debug() << "win reservation";
                mConflictedTransaction = reservation->transactionUUID();
                mStep = Cycles_WaitForOutgoingAmountReleasing;
                mOutgoingAmount = reservation->amount();
                return resultAwakeAfterMilliseconds(
                    kWaitingForReleasingAmountMSec);
            }
            debug() << "don't win reservation";
        }
    }

    if (!reserveOutgoingAmount(mNextNode, mOutgoingAmount, 0)) {
        mCyclesManager->addClosedTrustLine(
            currentNodeUUID(),
            mNextNode);
        return resultDone();
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendRequestToIntNodeOnReservationStage(
        mNextNode,
        mPathStats->maxFlow());
#endif

    debug() << "Send request reservation (" << mPathStats->maxFlow() << ") message to " << mNextNode;
    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        mNextNode,
        mEquivalent,
        kCurrentNode,
        kTransactionUUID,
        mPathStats->maxFlow(),
        currentNodeUUID(),
        mPathStats->path()->length());

    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse,
         Message::NoEquivalent},
        maxNetworkDelay(1));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runAmountReservationStageAgain()
{
    debug() << "runAmountReservationStageAgain";

    if (mCyclesManager->isTransactionStillAlive(
        mConflictedTransaction)) {
        debug() << "wait again";
        return resultAwakeAfterMilliseconds(
            kWaitingForReleasingAmountMSec);
    }

    debug() << "Reservation was released, continue";
    if (!reserveOutgoingAmount(mNextNode, mOutgoingAmount, 0)) {
        warning() << "Can't reserve. Close transaction";
        return resultDone();
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToReceiverOnReservationStage();
#endif

    mPathStats->shortageMaxFlow(mOutgoingAmount);
    debug() << "Send request reservation (" << mPathStats->maxFlow() << ") message to " << mNextNode;
    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        mNextNode,
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID(),
        mPathStats->maxFlow(),
        currentNodeUUID(),
        mPathStats->path()->length());

    mStep = Coordinator_AmountReservation;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse,
         Message::NoEquivalent},
        maxNetworkDelay(1));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::processNeighborAmountReservationResponse()
{
    debug() << "processNeighborAmountReservationResponse";
    if (contextIsValid(Message::NoEquivalent, false)) {
        warning() << "Neighbour hasn't TLs on requested equivalent. Canceling.";
        rollBack();
        return resultDone();
    }

    if (! contextIsValid(Message::Payments_IntermediateNodeCycleReservationResponse)) {
        debug() << "No neighbor node response received.";
        rollBack();
        mCyclesManager->addOfflineNode(mNextNode);
        return resultDone();
    }

    auto message = popNextMessage<IntermediateNodeCycleReservationResponseMessage>();
    // todo: check message sender

    if (message->state() == IntermediateNodeCycleReservationResponseMessage::Rejected ||
            message->state() == IntermediateNodeCycleReservationResponseMessage::RejectedBecauseReservations) {
        warning() << "Neighbor node doesn't approved reservation request";
        rollBack();
        if (message->state() == IntermediateNodeCycleReservationResponseMessage::Rejected) {
            mCyclesManager->addClosedTrustLine(
                currentNodeUUID(),
                mNextNode);
        }
        return resultDone();
    }

    if (message->state() != IntermediateNodeCycleReservationResponseMessage::Accepted) {
        warning() << "Unexpected message state. Protocol error.";
        rollBack();
        return resultDone();
    }

    debug() << "(" << message->senderUUID << ") approved reservation request.";
    mPathStats->setNodeState(
        1, PathStats::NeighbourReservationApproved);
    mPathStats->shortageMaxFlow(message->amountReserved());
    return runAmountReservationStage();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askNeighborToApproveFurtherNodeReservation()
{
    debug() << "askNeighborToApproveFurtherNodeReservation";
    const auto kCoordinator = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();
    const auto kNeighborPathPosition = 1;
    const auto kNextAfterNeighborNode = mPathStats->path()->nodes[kNeighborPathPosition+1];

    // Note:
    // no check of "neighbor" node is needed here.
    // It was done on previous step.

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
        mNextNode,
        mPathStats->maxFlow());
#endif

    sendMessage<CoordinatorCycleReservationRequestMessage>(
        mNextNode,
        mEquivalent,
        kCoordinator,
        kTransactionUUID,
        mPathStats->maxFlow(),
        kNextAfterNeighborNode);

    debug() << "Further amount reservation request sent to the node (" << mNextNode << ") [" << mPathStats->maxFlow() << "]";

    mPathStats->setNodeState(
        kNeighborPathPosition,
        PathStats::ReservationRequestSent);

    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationResponse},
        maxNetworkDelay(4));
}


TransactionResult::SharedConst CycleCloserInitiatorTransaction::processNeighborFurtherReservationResponse()
{
    debug() << "processNeighborFurtherReservationResponse";
    if (! contextIsValid(Message::Payments_CoordinatorCycleReservationResponse)) {
        debug() << "Neighbor node doesn't sent coordinator response.";
        rollBack();
        informIntermediateNodesAboutTransactionFinish(1);
        mCyclesManager->addOfflineNode(
            mNextNode);
        return resultDone();
    }

    /*
     * Nodes scheme:
     * R - remote node;
     */
    const auto R_UUIDAndPos = mPathStats->currentIntermediateNodeAndPos();
    const auto R_PathPosition = R_UUIDAndPos.second;

    auto message = popNextMessage<CoordinatorCycleReservationResponseMessage>();

    if (message->state() == IntermediateNodeCycleReservationResponseMessage::NextNodeInaccessible) {
        warning() << "Next node after neighbor is inaccessible.";
        rollBack();
        mCyclesManager->addOfflineNode(
            mPathStats->path()->nodes.at(
                R_PathPosition + 1));
        return resultDone();
    }

    if (message->state() == CoordinatorCycleReservationResponseMessage::Rejected ||
            message->state() == CoordinatorCycleReservationResponseMessage::RejectedBecauseReservations) {
        warning() << "Neighbor node doesn't accepted coordinator request.";
        rollBack();
        if (message->state() == CoordinatorCycleReservationResponseMessage::Rejected) {
            mCyclesManager->addClosedTrustLine(
                mNextNode,
                mPathStats->path()->nodes.at(R_PathPosition + 1));
        }
        return resultDone();
    }

    if (message->state() != IntermediateNodeCycleReservationResponseMessage::Accepted) {
        warning() << "Unexpected message state. Protocol error.";
        rollBack();
        return resultDone();
    }

    mPathStats->setNodeState(
        1,
        PathStats::ReservationApproved);
    debug() << "Neighbor node accepted coordinator request. Reserved: " << message->amountReserved();

    if (message->amountReserved() != mPathStats->maxFlow()) {
        mPathStats->shortageMaxFlow(message->amountReserved());
        debug() << "Path max flow is now " << mPathStats->maxFlow();
        for (auto const &itNodeAndReservations : mReservations) {
            auto nodeReservations = itNodeAndReservations.second;
            if (nodeReservations.size() != 1) {
                throw ValueError("CycleCloserInitiatorTransaction::processRemoteNodeResponse: "
                                     "unexpected behaviour: between two nodes should be only one reservation.");
            }
            shortageReservation(
                itNodeAndReservations.first,
                (*nodeReservations.begin()).second,
                mPathStats->maxFlow(),
                0);
        }
    }

    return runAmountReservationStage();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askRemoteNodeToApproveReservation(
    const NodeUUID& remoteNode,
    const byte remoteNodePosition,
    const NodeUUID& nextNodeAfterRemote)
{
    debug() << "askRemoteNodeToApproveReservation";
    const auto kCoordinator = currentNodeUUID();
    const auto kTransactionUUID = currentTransactionUUID();

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
        remoteNode,
        mPathStats->maxFlow());
#endif

    sendMessage<CoordinatorCycleReservationRequestMessage>(
        remoteNode,
        mEquivalent,
        kCoordinator,
        kTransactionUUID,
        mPathStats->maxFlow(),
        nextNodeAfterRemote);

    mPathStats->setNodeState(
        remoteNodePosition,
        PathStats::ReservationRequestSent);

    if (mPathStats->isLastIntermediateNodeProcessed()) {
        debug() << "Further amount reservation request sent to the last node (" << remoteNode << ") ["
               << mPathStats->maxFlow() << ", next node - (" << nextNodeAfterRemote << ")]";
        mStep = Coordinator_PreviousNeighborRequestProcessing;
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeCycleReservationRequest,
            Message::Payments_CoordinatorCycleReservationResponse},
            maxNetworkDelay(2));
    }
    debug() << "Further amount reservation request sent to the node (" << remoteNode << ") ["
           << mPathStats->maxFlow() << ", next node - (" << nextNodeAfterRemote << ")]";

    // Response from te remote node will go throught other nodes in the path.
    // So them would be able to shortage it's reservations (if needed).
    // Total wait timeout must take note of this.
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationResponse},
        maxNetworkDelay(4));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::processRemoteNodeResponse()
{
    debug() << "processRemoteNodeResponse";
    if (!contextIsValid(Message::Payments_CoordinatorCycleReservationResponse)){
        warning() << "Remote node doesn't sent coordinator response. Can't pay.";
        informIntermediateNodesAboutTransactionFinish(
            mPathStats->currentIntermediateNodeAndPos().second);
        mCyclesManager->addOfflineNode(
            mPathStats->currentIntermediateNodeAndPos().first);
        return resultDone();
    }

    const auto message = popNextMessage<CoordinatorCycleReservationResponseMessage>();

    /*
     * Nodes scheme:
     * R - remote node;
     */
    const auto R_UUIDAndPos = mPathStats->currentIntermediateNodeAndPos();
    const auto R_PathPosition = R_UUIDAndPos.second;

    if (message->state() == CoordinatorCycleReservationResponseMessage::NextNodeInaccessible) {
        warning() << "Next node after neighbor is inaccessible.";
        rollBack();
        informIntermediateNodesAboutTransactionFinish(
            mPathStats->currentIntermediateNodeAndPos().second - 1);
        if (mPathStats->path()->nodes.at(R_PathPosition + 1) != mNodeUUID) {
            mCyclesManager->addOfflineNode(
                mPathStats->path()->nodes.at(
                    R_PathPosition + 1));
        }
        return resultDone();
    }

    if (message->state() == CoordinatorCycleReservationResponseMessage::Rejected ||
            message->state() == CoordinatorCycleReservationResponseMessage::RejectedBecauseReservations) {
        warning() << "Remote node rejected reservation. Can't pay";
        rollBack();
        informIntermediateNodesAboutTransactionFinish(
            mPathStats->currentIntermediateNodeAndPos().second - 1);
        if (message->state() == CoordinatorCycleReservationResponseMessage::Rejected) {
            mCyclesManager->addClosedTrustLine(
                mPathStats->path()->nodes.at(R_PathPosition),
                mPathStats->path()->nodes.at(R_PathPosition + 1));
        }
        return resultDone();
    }

    if (message->state() != IntermediateNodeCycleReservationResponseMessage::Accepted) {
        warning() << "Unexpected message state. Protocol error.";
        rollBack();
        return resultDone();
    }

    const auto reservedAmount = message->amountReserved();
    debug() << "(" << message->senderUUID << ") reserved " << reservedAmount;

    mPathStats->setNodeState(
        R_PathPosition,
        PathStats::ReservationApproved);

    if (reservedAmount != mPathStats->maxFlow()) {
        mPathStats->shortageMaxFlow(reservedAmount);
        for (auto const &itNodeAndReservations : mReservations) {
            auto nodeReservations = itNodeAndReservations.second;
            if (nodeReservations.size() != 1) {
                throw ValueError("CycleCloserInitiatorTransaction::processRemoteNodeResponse: "
                                     "unexpected behaviour: between two nodes should be only one reservation.");
            }
            shortageReservation(
                itNodeAndReservations.first,
                (*nodeReservations.begin()).second,
                mPathStats->maxFlow(),
                0);
        }
        debug() << "Path max flow is now " << mPathStats->maxFlow();
    }

    if (mPathStats->isLastIntermediateNodeProcessed()) {

        const auto kTotalAmount = mPathStats->maxFlow();
        debug() << "Current path reservation finished";
        debug() << "Total collected amount by cycle: " << kTotalAmount;
        debug() << "Total count of all participants without coordinator is " << mPathStats->path()->intermediateUUIDs().size();

#ifdef TESTS
        mSubsystemsController->testForbidSendMessageWithFinalPathConfiguration(
            mPathStats->path()->intermediateUUIDs().size());
#endif

        // send final path amount to all intermediate nodes on path
        if (!sendFinalPathConfiguration(
            mPathStats->maxFlow())) {
            return reject("Can't send final path configuration. Rejected.");
        }

        mAllNodesSentConfirmationOnFinalAmountsConfiguration = false;
        mAllNeighborsSentFinalReservations = false;

        mStep = Coordinator_FinalAmountsConfigurationConfirmation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfigurationResponse,
             Message::Payments_TransactionPublicKeyHash},
            maxNetworkDelay(3));
    }

    debug() << "Go to the next node in path";
    return tryReserveNextIntermediateNodeAmount();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runPreviousNeighborRequestProcessingStage()
{
    debug() << "runPreviousNeighborRequestProcessingStage";
    if (! contextIsValid(Message::Payments_IntermediateNodeCycleReservationRequest, false))
        return reject("No amount reservation request was received. Rolled back.");

    const auto kMessage = popNextMessage<IntermediateNodeCycleReservationRequestMessage>();
    mPreviousNode = kMessage->senderUUID;
    debug() << "Coordinator payment operation from node (" << mPreviousNode << ")";
    debug() << "Requested amount reservation: " << kMessage->amount();

    const auto kIncomingAmounts = mTrustLines->availableIncomingCycleAmounts(mPreviousNode);
    const auto kIncomingAmountWithReservations = kIncomingAmounts.first;
    const auto kIncomingAmountWithoutReservations = kIncomingAmounts.second;
    debug() << "IncomingAmountWithReservations: " << *kIncomingAmountWithReservations
            << " IncomingAmountWithoutReservations: " << *kIncomingAmountWithoutReservations;
    if (*kIncomingAmountWithReservations == TrustLine::kZeroAmount()) {
        if (*kIncomingAmountWithoutReservations == TrustLine::kZeroAmount()) {
            sendMessage<IntermediateNodeCycleReservationResponseMessage>(
                mPreviousNode,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                ResponseCycleMessage::Rejected);
            warning() << "can't reserve requested amount, because coordinator incoming amount "
                "event without reservations equal zero, transaction closed";
            return resultDone();
        } else {
            mIncomingAmount = TrustLineAmount(0);
        }
    } else {
        mIncomingAmount = min(
            kMessage->amount(),
            *kIncomingAmountWithReservations);
    }

    if (0 == mIncomingAmount) {
        auto reservations = mTrustLines->reservationsFromContractor(mPreviousNode);
        for (auto &reservation : reservations) {
            if (mCyclesManager->resolveReservationConflict(
                currentTransactionUUID(), reservation->transactionUUID())) {
                mConflictedTransaction = reservation->transactionUUID();
                mStep = Cycles_WaitForIncomingAmountReleasing;
                mIncomingAmount = min(
                    reservation->amount(),
                    kMessage->amount());
                return resultAwakeAfterMilliseconds(
                    kWaitingForReleasingAmountMSec);
            }
        }
    }

    if (0 == mIncomingAmount || ! reserveIncomingAmount(mPreviousNode, mIncomingAmount, 0)) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            mPreviousNode,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::RejectedBecauseReservations);
        return reject("No incoming amount reservation is possible. Rolled back.");
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
        kMessage->senderUUID,
        mIncomingAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    debug() << "send accepted message with reserve (" << mIncomingAmount << ") to node " << mPreviousNode;
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID(),
        ResponseCycleMessage::Accepted,
        mIncomingAmount);

    mStep = Coordinator_AmountReservation;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runPreviousNeighborRequestProcessingStageAgain()
{
    debug() << "runPreviousNeighborRequestProcessingStageAgain";

    if (mCyclesManager->isTransactionStillAlive(
        mConflictedTransaction)) {
        debug() << "wait again";
        return resultAwakeAfterMilliseconds(
            kWaitingForReleasingAmountMSec);
    }

    if (! reserveIncomingAmount(mPreviousNode, mIncomingAmount, 0)) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            mPreviousNode,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::RejectedBecauseReservations);
        return reject("No incoming amount reservation is possible. Rolled back.");
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
        mPreviousNode,
        mIncomingAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    debug() << "send accepted message with reserve (" << mIncomingAmount << ") to node " << mPreviousNode;
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID(),
        ResponseCycleMessage::Accepted,
        mIncomingAmount);

    mStep = Coordinator_AmountReservation;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runFinalAmountsConfigurationConfirmationProcessingStage()
{
    debug() << "runFinalAmountsConfigurationConfirmationProcessingStage";
    if (contextIsValid(Message::Payments_FinalAmountsConfigurationResponse, false)) {
        return runFinalAmountsParticipantConfirmation();
    }

    if (contextIsValid(Message::Payments_TransactionPublicKeyHash, false)) {
        return runFinalReservationsNeighborConfirmation();
    }

    return reject("Some nodes didn't confirm final amount configuration. Transaction rejected.");
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runFinalAmountsParticipantConfirmation()
{
    debug() << "runFinalAmountsParticipantConfirmation";
    auto kMessage = popNextMessage<FinalAmountsConfigurationResponseMessage>();
    debug() << "sender: " << kMessage->senderUUID;
    if (mPaymentNodesIds.find(kMessage->senderUUID) == mPaymentNodesIds.end()) {
        warning() << "Sender is not participant of this transaction";
        return resultContinuePreviousState();
    }
    if (kMessage->state() == FinalAmountsConfigurationResponseMessage::Rejected) {
        return reject("Haven't reach consensus on reservation. Transaction rejected.");
    }
    debug() << "Node " << kMessage->senderUUID << " confirmed final amounts";
    mParticipantsPublicKeys[mPaymentNodesIds[kMessage->senderUUID]] = kMessage->publicKey();

    if (mParticipantsPublicKeys.size() + 1 < mPaymentNodesIds.size()) {
        debug() << "Some nodes are still not confirmed final amounts. Waiting.";
        if (mAllNeighborsSentFinalReservations) {
            return resultWaitForMessageTypes(
                {Message::Payments_FinalAmountsConfigurationResponse},
                maxNetworkDelay(1));
        } else {
            return resultWaitForMessageTypes(
                {Message::Payments_FinalAmountsConfigurationResponse,
                 Message::Payments_TransactionPublicKeyHash},
                maxNetworkDelay(1));
        }
    }

    debug() << "All nodes confirmed final configuration.";
    if (mAllNeighborsSentFinalReservations) {
        debug() << "Begin processing participants votes.";
        return propagateVotesListAndWaitForVotingResult();
    } else {
        return resultWaitForMessageTypes(
            {Message::Payments_TransactionPublicKeyHash},
            maxNetworkDelay(1));
    }
}

// todo customize this logic on CycleCloserInitiator
TransactionResult::SharedConst CycleCloserInitiatorTransaction::runFinalReservationsNeighborConfirmation()
{
    debug() << "runFinalReservationsNeighborConfirmation";
    auto kMessage = popNextMessage<TransactionPublicKeyHashMessage>();
    debug() << "sender: " << kMessage->senderUUID;

    mParticipantsPublicKeysHashes[kMessage->senderUUID] = make_pair(
        kMessage->paymentNodeID(),
        kMessage->transactionPublicKeyHash());

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (kMessage->isReceiptContains()) {
        auto participantTotalIncomingReservationAmount = totalReservedIncomingAmountToNode(
            kMessage->senderUUID);
        info() << "Sender also send receipt";

        auto keyChain = mKeysStore->keychain(
            mTrustLines->trustLineID(kMessage->senderUUID));
        auto serializedIncomingReceiptData = getSerializedReceipt(
            kMessage->senderUUID,
            mNodeUUID,
            participantTotalIncomingReservationAmount);
        if (!keyChain.checkSign(
            ioTransaction,
            serializedIncomingReceiptData.first,
            serializedIncomingReceiptData.second,
            kMessage->signature(),
            kMessage->publicKeyNumber())) {
            return reject("Sender send invalid receipt signature. Rejected");
        }
        if (!keyChain.saveIncomingPaymentReceipt(
            ioTransaction,
            mTrustLines->auditNumber(kMessage->senderUUID),
            mTransactionUUID,
            kMessage->publicKeyNumber(),
            participantTotalIncomingReservationAmount,
            kMessage->signature())) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            return reject("Can't save participant receipt. Rejected.");
        }
        info() << "Sender's receipt is valid";
        mAllNeighborsSentFinalReservations = true;
    } else {
        // coordinator should receive only 1 message from last intermediate node
        // and this message should contain receipt
        return reject("Sender send message without receipt. Rejected");
    }

    // only one node may send reservations
    if (mAllNeighborsSentFinalReservations) {
        info() << "All neighbors sent theirs reservations";
        if (mAllNodesSentConfirmationOnFinalAmountsConfiguration) {
            debug() << "Begin processing participants votes.";
            return propagateVotesListAndWaitForVotingResult();
        }
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfigurationResponse},
            maxNetworkDelay(1));
    }

    // not all neighbors sent theirs reservations
    if (mAllNodesSentConfirmationOnFinalAmountsConfiguration) {
        return resultWaitForMessageTypes(
            {Message::Payments_TransactionPublicKeyHash},
            maxNetworkDelay(1));
    } else {
        return resultWaitForMessageTypes(
            {Message::Payments_TransactionPublicKeyHash,
             Message::Payments_FinalAmountsConfigurationResponse},
            maxNetworkDelay(1));
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runVotesConsistencyCheckingStage()
{
    debug() << "runVotesConsistencyCheckingStage";
    if (! contextIsValid(Message::Payments_ParticipantVote)) {
        return reject("Coordinator didn't receive all messages with votes");
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnVoteConsistencyStage(
        mPaymentNodesIds.size());
    mSubsystemsController->testThrowExceptionOnVoteConsistencyStage();
    mSubsystemsController->testTerminateProcessOnVoteConsistencyStage();
#endif

    const auto kMessage = popNextMessage<ParticipantVoteMessage>();
    debug () << "Participant vote message received from " << kMessage->senderUUID;

    if (mPaymentNodesIds.find(kMessage->senderUUID) == mPaymentNodesIds.end()) {
        warning() << "Sender is not participant of current transaction";
        return resultContinuePreviousState();
    }

    auto participantSign = kMessage->signature();
    auto participantPublicKey = mParticipantsPublicKeys[mPaymentNodesIds[kMessage->senderUUID]];
    auto participantSerializedVotesData = getSerializedParticipantsVotesData(
        kMessage->senderUUID);
    // todo if we store participants public keys on database, then we should use KeyChain,
    // or we can check sign directly from mParticipantsPublicKeys
    if (!participantSign->check(
            participantSerializedVotesData.first.get(),
            participantSerializedVotesData.second,
            participantPublicKey)) {
        return reject("Participant signature is incorrect. Rolling back");
    }
    info() << "Participant signature is correct";
    mParticipantsSignatures.insert(make_pair(
        mPaymentNodesIds[kMessage->senderUUID],
        participantSign));

    if (mParticipantsSignatures.size() + 1 == mPaymentNodesIds.size()) {
        info() << "all participants sign their data";

        auto serializedOwnVotesData = getSerializedParticipantsVotesData(
            mNodeUUID);
        {
            auto ioTransaction = mStorageHandler->beginTransaction();
            auto ownSignature = mKeysStore->signPaymentTransaction(
                ioTransaction,
                currentTransactionUUID(),
                serializedOwnVotesData.first,
                serializedOwnVotesData.second);
            mParticipantsSignatures.insert(
                make_pair(
                    mPaymentNodesIds[mNodeUUID],
                    ownSignature));
        }
        debug() << "Voted +";
        mParticipantsVotesMessage = make_shared<ParticipantsVotesMessage>(
            mEquivalent,
            mNodeUUID,
            currentTransactionUUID(),
            mParticipantsSignatures);
        return approve();
    }

    info() << "Not all participants send theirs signatures";
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantVote},
        maxNetworkDelay(3));
}

void CycleCloserInitiatorTransaction::checkPath(
    const Path::ConstShared path)
{
    if (path->length() < 3 || path->length() > 7) {
        throw ValueError("CycleCloserInitiatorTransaction::checkPath: "
                             "invalid paths length");
    }
    if (path->sourceUUID() != path->destinationUUID()) {
        throw ValueError("CycleCloserInitiatorTransaction::checkPath: "
                             "path isn't cycle");
    }
    for (const auto &node : path->intermediateUUIDs()) {
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

void CycleCloserInitiatorTransaction::informIntermediateNodesAboutTransactionFinish(
    const SerializedPositionInPath lastInformedNodePosition)
{
    debug() << "informIntermediateNodesAboutTransactionFinish";
    for (SerializedPositionInPath nodePosition = 1; nodePosition <= lastInformedNodePosition; nodePosition++) {
        sendMessage<TTLProlongationResponseMessage>(
            mPathStats->path()->nodes.at(nodePosition),
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Finish);
        debug() << "send message with transaction finishing instruction to node "
                << mPathStats->path()->nodes.at(nodePosition);
    }
}

bool CycleCloserInitiatorTransaction::sendFinalPathConfiguration(
    const TrustLineAmount &finalPathAmount)
{
    debug() << "sendFinalPathConfiguration";
    mPaymentNodesIds.insert(
        make_pair(
            mNodeUUID,
            kCoordinatorPaymentNodeID));
    PaymentNodeID currentNodeID = kCoordinatorPaymentNodeID + 1;
    for (const auto &intermediateNode : mPathStats->path()->intermediateUUIDs()) {
        mPaymentNodesIds.insert(
            make_pair(
                intermediateNode,
                currentNodeID++));
    }
    bool sendReservationToFirstIntermediateNode = true;
    for (const auto &intermediateNode : mPathStats->path()->intermediateUUIDs()) {
        if (sendReservationToFirstIntermediateNode) {
            // we should send receipt to first intermediate node
            auto keyChain = mKeysStore->keychain(
                mTrustLines->trustLineID(intermediateNode));
            // coordinator should have one outgoing reservation to first intermediate node
            auto serializedOutgoingReceiptData = getSerializedReceipt(
                mNodeUUID,
                intermediateNode,
                finalPathAmount);
            auto ioTransaction = mStorageHandler->beginTransaction();
            auto signatureAndKeyNumber = keyChain.sign(
                ioTransaction,
                serializedOutgoingReceiptData.first,
                serializedOutgoingReceiptData.second);
            if (!keyChain.saveOutgoingPaymentReceipt(
                    ioTransaction,
                    mTrustLines->auditNumber(intermediateNode),
                    mTransactionUUID,
                    signatureAndKeyNumber.second,
                    finalPathAmount,
                    signatureAndKeyNumber.first)) {
                warning() << "Can't save outgoing receipt.";
                return false;
            }
            info() << "send message with final path amount info for node "
                   << intermediateNode << " with receipt";
            sendMessage<FinalPathCycleConfigurationMessage>(
                intermediateNode,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                finalPathAmount,
                mPaymentNodesIds,
                signatureAndKeyNumber.second,
                signatureAndKeyNumber.first);
            sendReservationToFirstIntermediateNode = false;
        } else {
            info() << "send message with final path amount info for node " << intermediateNode;
            sendMessage<FinalPathCycleConfigurationMessage>(
                intermediateNode,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                finalPathAmount,
                mPaymentNodesIds);
        }
    }
    return true;
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::approve()
{
    mCommittedAmount = totalReservedAmount(
        AmountReservation::Outgoing);
    BasePaymentTransaction::approve();
    for (const auto &nodeUUIDAndPaymentNodeID : mPaymentNodesIds) {
        if (nodeUUIDAndPaymentNodeID.first == mNodeUUID) {
            continue;
        }
        sendMessage(
            nodeUUIDAndPaymentNodeID.first,
            mParticipantsVotesMessage);
    }
    return resultDone();
}

void CycleCloserInitiatorTransaction::savePaymentOperationIntoHistory(
    IOTransaction::Shared ioTransaction)
{
    debug() << "savePaymentOperationIntoHistory";
    ioTransaction->historyStorage()->savePaymentRecord(
        make_shared<PaymentRecord>(
            currentTransactionUUID(),
            PaymentRecord::PaymentOperationType::CycleCloserType,
            mCommittedAmount),
        mEquivalent);
    debug() << "Operation saved";
}

bool CycleCloserInitiatorTransaction::checkReservationsDirections() const
{
    debug() << "checkReservationsDirections";
    if (mReservations.size() != 2) {
        warning() << "Wrong nodes reservations size: " << mReservations.size();
        return false;
    }

    auto firstNodeReservation = mReservations.begin()->second;
    auto secondNodeReservation = mReservations.rbegin()->second;
    if (firstNodeReservation.size() != 1 || secondNodeReservation.size() != 1) {
        warning() << "Wrong reservations size";
        return false;
    }
    const auto firstReservation = firstNodeReservation.at(0);
    const auto secondReservation = secondNodeReservation.at(0);
    if (firstReservation.first != secondReservation.first) {
        warning() << "Reservations on different ways";
        return false;
    }
    if (firstReservation.second->amount() != secondReservation.second->amount()) {
        warning() << "Different reservations amount";
        return false;
    }
    if (firstReservation.second->direction() == secondReservation.second->direction()) {
        warning() << "Wrong directions";
        return false;
    }
    return true;
}

const NodeUUID& CycleCloserInitiatorTransaction::coordinatorUUID() const
{
    return currentNodeUUID();
}

const SerializedPathLengthSize CycleCloserInitiatorTransaction::cycleLength() const
{
    return (SerializedPathLengthSize)mPathStats->path()->length();
}

const string CycleCloserInitiatorTransaction::logHeader() const
{
    stringstream s;
    s << "[CycleCloserInitiatorTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}