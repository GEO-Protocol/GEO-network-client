#include "CycleCloserInitiatorTransaction.h"

CycleCloserInitiatorTransaction::CycleCloserInitiatorTransaction(
    const Path::Shared path,
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
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
        equivalent,
        false,
        contractorsManager,
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
    // todo : correct method checkPath
    checkPath(
        mPathStats->path());
    debug() << "cycle is valid";
    mNextNode = mPathStats->path()->intermediates()[0];
    mNextNodeID = mContractorsManager->contractorIDByAddress(mNextNode);
    if (mNextNodeID == ContractorsManager::kNotFoundContractorID) {
        warning() << "First node in cycle is not neighbor " << mNextNode->fullAddress();
        return resultDone();
    }
    debug() << "first intermediate node: " << mNextNode->fullAddress() << " contractorID " << mNextNodeID;
    if (! mTrustLinesManager->trustLineIsPresent(mNextNodeID)){
        // Internal process error. Wrong path
        warning() << "Invalid path occurred. Node (" << mNextNode->fullAddress()
                  << ") is not listed in first level contractors list.";
        throw RuntimeError(
            "CycleCloserInitiatorTransaction::runAmountReservationStage: "
                "invalid first level node occurred. ");
    }

    const auto kOutgoingAmounts = mTrustLinesManager->availableOutgoingCycleAmounts(mNextNodeID);
    const auto kOutgoingAmountWithReservations = kOutgoingAmounts.first;
    const auto kOutgoingAmountWithoutReservations = kOutgoingAmounts.second;
    debug() << "OutgoingAmountWithReservations: " << *kOutgoingAmountWithReservations
            << " OutgoingAmountWithoutReservations: " << *kOutgoingAmountWithoutReservations;

    if (*kOutgoingAmountWithReservations == TrustLine::kZeroAmount()) {
        if (*kOutgoingAmountWithoutReservations == TrustLine::kZeroAmount()) {
            warning() << "Can't close cycle, because coordinator outgoing amount equal zero, "
                "and can't use reservations from other transactions";
            mCyclesManager->addClosedTrustLine(
                mContractorsManager->ownAddresses().at(0),
                mNextNode);
            return resultDone();
        } else {
            mOutgoingAmount = TrustLineAmount(0);
        }
    } else {
        mOutgoingAmount = *kOutgoingAmountWithReservations;
    }

    debug() << "outgoing Possibilities: " << mOutgoingAmount;

    mPreviousNode = mPathStats->path()->intermediates()[mPathStats->path()->length() - 1];
    mPreviousNodeID = mContractorsManager->contractorIDByAddress(mPreviousNode);
    if (mPreviousNodeID == ContractorsManager::kNotFoundContractorID) {
        warning() << "last intermediate node is not a neighbor " << mPreviousNode->fullAddress();
    }
    debug() << "last intermediate node: " << mPreviousNode->fullAddress() << " contractorID " << mPreviousNodeID;
    if (! mTrustLinesManager->trustLineIsPresent(mPreviousNodeID)){
        // Internal process error. Wrong path
        warning() << "Invalid path occurred. Node (" << mPreviousNode->fullAddress()
                  << ") is not listed in first level contractors list.";
        throw RuntimeError(
            "CycleCloserInitiatorTransaction::runAmountReservationStage: "
                "invalid first level node occurred. ");
    }

    const auto kIncomingAmounts = mTrustLinesManager->availableIncomingCycleAmounts(mPreviousNodeID);
    mIncomingAmount = *(kIncomingAmounts.first);

    if (mIncomingAmount == TrustLine::kZeroAmount()) {
        warning() << "Can't close cycle, because coordinator incoming amount equal zero";
        mCyclesManager->addClosedTrustLine(
            mPreviousNode,
            mContractorsManager->ownAddresses().at(0));
        return resultDone();
    }
    debug() << "Incoming Possibilities: " << mIncomingAmount;

    if (mIncomingAmount < mOutgoingAmount) {
        mOutgoingAmount = mIncomingAmount;
    }
    debug() << "Initial Outgoing Amount: " << mOutgoingAmount;
    mPathStats->path()->addReceiver(
        mContractorsManager->ownAddresses().at(0));
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
        currentTransactionUUID());
    mParticipantsPublicKeys.insert(
        make_pair(
            kCoordinatorPaymentNodeID,
            mPublicKey));

    // send message with all public keys to all participants and wait for voting results
    for (const auto &paymentIdAndAddress : mPaymentParticipants) {
        if (paymentIdAndAddress.first == kCoordinatorPaymentNodeID) {
            continue;
        }
        sendMessage<ParticipantsPublicKeysMessage>(
            paymentIdAndAddress.second,
            mEquivalent,
            mContractorsManager->ownAddresses(),
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
    try {
        const auto remoteAddressAndPos = mPathStats->nextIntermediateNodeAndPos();
        const auto remoteAddress = remoteAddressAndPos.first;
        const auto remoteNodePositionInPath = remoteAddressAndPos.second;

        if (remoteNodePositionInPath == 0) {
            if (mPathStats->isNeighborAmountReserved()) {
                return askNeighborToApproveFurtherNodeReservation();
            } else
                return askNeighborToReserveAmount();

        } else {
            debug() << "Processing " << int(remoteNodePositionInPath)
                    << " node in path: (" << remoteAddress->fullAddress() << ").";

            const auto nextAfterRemoteNode = mPathStats->path()->intermediates()[remoteNodePositionInPath + 1];
            return askRemoteNodeToApproveReservation(
                remoteAddress,
                remoteNodePositionInPath,
                nextAfterRemoteNode);
        }

    } catch(NotFoundError) {
        warning() << "No unprocessed paths are left. Requested amount can't be collected. Canceling.";
        return resultDone();
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askNeighborToReserveAmount()
{
    debug() << "askNeighborToReserveAmount";

    // todo maybe check in storage (keyChain)
    if (!mTrustLinesManager->trustLineOwnKeysPresent(mNextNodeID)) {
        warning() << "There are no own keys on TL with contractor " << mNextNode->fullAddress();
        mCyclesManager->addClosedTrustLine(
            mContractorsManager->ownAddresses().at(0),
            mNextNode);
        return resultDone();
    }

    // Try reserve amount locally.
    mPathStats->shortageMaxFlow(mOutgoingAmount);
    mPathStats->setNodeState(
        0,
        PathStats::NeighbourReservationRequestSent);

    if (0 == mOutgoingAmount) {
        // try use reservations from other transactions
        auto reservations = mTrustLinesManager->reservationsToContractor(mNextNodeID);
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

    if (!reserveOutgoingAmount(mNextNodeID, mOutgoingAmount, 0)) {
        mCyclesManager->addClosedTrustLine(
            mContractorsManager->ownAddresses().at(0),
            mNextNode);
        return resultDone();
    }

#ifdef TESTS
    // todo : adapt SubsystemsController
//    mSubsystemsController->testForbidSendRequestToIntNodeOnReservationStage(
//        mNextNode,
//        mPathStats->maxFlow());
#endif

    debug() << "Send request reservation (" << mPathStats->maxFlow() << ") message to next node";
    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        mNextNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        mPathStats->maxFlow(),
        mContractorsManager->ownAddresses().at(0),
        mPathStats->path()->length());

    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse,
         Message::General_NoEquivalent},
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
    if (!reserveOutgoingAmount(mNextNodeID, mOutgoingAmount, 0)) {
        warning() << "Can't reserve. Close transaction";
        return resultDone();
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToReceiverOnReservationStage();
#endif

    mPathStats->shortageMaxFlow(mOutgoingAmount);
    debug() << "Send request reservation (" << mPathStats->maxFlow() << ") message to next node";
    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        mNextNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        mPathStats->maxFlow(),
        mContractorsManager->ownAddresses().at(0),
        mPathStats->path()->length());

    mStep = Coordinator_AmountReservation;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse,
         Message::General_NoEquivalent},
        maxNetworkDelay(1));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::processNeighborAmountReservationResponse()
{
    debug() << "processNeighborAmountReservationResponse";
    if (contextIsValid(Message::General_NoEquivalent, false)) {
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
    if (message->senderAddresses.at(0) != mNextNode) {
        warning() << "Sender is not next node " << message->senderAddresses.at(0)->fullAddress();
        return resultContinuePreviousState();
    }

    if (message->state() == IntermediateNodeCycleReservationResponseMessage::Rejected ||
            message->state() == IntermediateNodeCycleReservationResponseMessage::RejectedBecauseReservations) {
        warning() << "Neighbor node doesn't approved reservation request";
        rollBack();
        if (message->state() == IntermediateNodeCycleReservationResponseMessage::Rejected) {
            mCyclesManager->addClosedTrustLine(
                mContractorsManager->ownAddresses().at(0),
                mNextNode);
        }
        return resultDone();
    }

    if (message->state() == IntermediateNodeCycleReservationResponseMessage::RejectedDueContractorKeysAbsence) {
        warning() << "Neighbor node doesn't approved reservation request due to contractor keys absence";
        rollBack();
        mCyclesManager->addClosedTrustLine(
            mContractorsManager->ownAddresses().at(0),
            mNextNode);
        // todo maybe set mOwnKeysPresent into false and initiate KeysSharing TA
        return resultDone();
    }

    if (message->state() != IntermediateNodeCycleReservationResponseMessage::Accepted) {
        warning() << "Unexpected message state. Protocol error.";
        rollBack();
        return resultDone();
    }

    debug() << "Next node approved reservation request.";
    mPathStats->setNodeState(
        0, PathStats::NeighbourReservationApproved);
    mPathStats->shortageMaxFlow(message->amountReserved());
    return runAmountReservationStage();
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::askNeighborToApproveFurtherNodeReservation()
{
    debug() << "askNeighborToApproveFurtherNodeReservation";
    const auto kNextAfterNeighborNode = mPathStats->path()->intermediates()[1];

    // Note:
    // no check of "neighbor" node is needed here.
    // It was done on previous step.

#ifdef TESTS
    // todo : adapt SubsystemsController
//    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
//        mNextNode,
//        mPathStats->maxFlow());
#endif

    sendMessage<CoordinatorCycleReservationRequestMessage>(
        mNextNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        mPathStats->maxFlow(),
        kNextAfterNeighborNode);

    debug() << "Further amount reservation request sent to the next node [" << mPathStats->maxFlow()
            << "] next node - (" << kNextAfterNeighborNode->fullAddress() << ")]";

    mPathStats->setNodeState(
        0,
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

    auto message = popNextMessage<CoordinatorCycleReservationResponseMessage>();
    if (message->senderAddresses.at(0) != mNextNode) {
        warning() << "Sender is not next node " << message->senderAddresses.at(0)->fullAddress();
        return resultContinuePreviousState();
    }
    const auto kNextAfterNeighborNode = mPathStats->path()->intermediates()[1];

    if (message->state() == IntermediateNodeCycleReservationResponseMessage::NextNodeInaccessible) {
        warning() << "Next node after neighbor is inaccessible.";
        rollBack();
        mCyclesManager->addOfflineNode(
            kNextAfterNeighborNode);
        return resultDone();
    }

    if (message->state() == CoordinatorCycleReservationResponseMessage::Rejected ||
            message->state() == CoordinatorCycleReservationResponseMessage::RejectedBecauseReservations ||
            message->state() == CoordinatorCycleReservationResponseMessage::RejectedDueOwnKeysAbsence ||
            message->state() == CoordinatorCycleReservationResponseMessage::RejectedDueContractorKeysAbsence) {
        warning() << "Neighbor node doesn't accepted coordinator request.";
        rollBack();
        if (message->state() == CoordinatorCycleReservationResponseMessage::Rejected) {
            mCyclesManager->addClosedTrustLine(
                mNextNode,
                kNextAfterNeighborNode);
        }
        return resultDone();
    }

    if (message->state() != IntermediateNodeCycleReservationResponseMessage::Accepted) {
        warning() << "Unexpected message state. Protocol error.";
        rollBack();
        return resultDone();
    }

    mPathStats->setNodeState(
        0,
        PathStats::ReservationApproved);
    debug() << "Neighbor node accepted coordinator request. Reserved: " << message->amountReserved();

    if (message->amountReserved() != mPathStats->maxFlow()) {
        mPathStats->shortageMaxFlow(message->amountReserved());
        debug() << "Path max flow is now " << mPathStats->maxFlow();
        // todo : inspect this peace of code
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
    BaseAddress::Shared remoteNode,
    const byte remoteNodePosition,
    BaseAddress::Shared nextNodeAfterRemote)
{
    debug() << "askRemoteNodeToApproveReservation";

#ifdef TESTS
    // todo : adapt SubsystemsController
//    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
//        remoteNode,
//        mPathStats->maxFlow());
#endif

    sendMessage<CoordinatorCycleReservationRequestMessage>(
        remoteNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        mPathStats->maxFlow(),
        nextNodeAfterRemote);

    mPathStats->setNodeState(
        remoteNodePosition,
        PathStats::ReservationRequestSent);

    if (mPathStats->isLastIntermediateNodeProcessed()) {
        debug() << "Further amount reservation request sent to the last node (" << remoteNode->fullAddress() << ") ["
               << mPathStats->maxFlow() << ", next node - (" << nextNodeAfterRemote->fullAddress() << ")]";
        mStep = Coordinator_PreviousNeighborRequestProcessing;
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeCycleReservationRequest,
            Message::Payments_CoordinatorCycleReservationResponse},
            maxNetworkDelay(2));
    }
    debug() << "Further amount reservation request sent to the node (" << remoteNode->fullAddress() << ") ["
           << mPathStats->maxFlow() << ", next node - (" << nextNodeAfterRemote->fullAddress() << ")]";

    // Response from te remote node will go through other nodes in the path.
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
        rollBack();
        informIntermediateNodesAboutTransactionFinish(
            mPathStats->currentIntermediateNodeAndPos().second);
        mCyclesManager->addOfflineNode(
            mPathStats->currentIntermediateNodeAndPos().first);
        return resultDone();
    }

    const auto message = popNextMessage<CoordinatorCycleReservationResponseMessage>();
    auto messageSender = message->senderAddresses.at(0);
    info() << "Node " << messageSender->fullAddress() << " sent response";
    // todo : check is sender is the same node which was requested

    const auto remoteNodeAddressAndPos = mPathStats->currentIntermediateNodeAndPos();
    const auto remoteNodePathPosition = remoteNodeAddressAndPos.second;

    if (message->state() == CoordinatorCycleReservationResponseMessage::NextNodeInaccessible) {
        warning() << "Next node after neighbor is inaccessible.";
        rollBack();
        informIntermediateNodesAboutTransactionFinish(
            remoteNodePathPosition - 1);
        if (mPathStats->path()->intermediates().at(remoteNodePathPosition + 1) !=
                mContractorsManager->ownAddresses().at(0)) {
            mCyclesManager->addOfflineNode(
                mPathStats->path()->intermediates().at(
                    remoteNodePathPosition + 1));
        }
        return resultDone();
    }

    if (message->state() == CoordinatorCycleReservationResponseMessage::Rejected ||
            message->state() == CoordinatorCycleReservationResponseMessage::RejectedBecauseReservations ||
            message->state() == CoordinatorCycleReservationResponseMessage::RejectedDueOwnKeysAbsence ||
            message->state() == CoordinatorCycleReservationResponseMessage::RejectedDueContractorKeysAbsence) {
        warning() << "Remote node rejected reservation. Can't pay";
        rollBack();
        informIntermediateNodesAboutTransactionFinish(
            remoteNodePathPosition - 1);
        if (message->state() == CoordinatorCycleReservationResponseMessage::Rejected) {
            mCyclesManager->addClosedTrustLine(
                mPathStats->path()->intermediates().at(remoteNodePathPosition),
                mPathStats->path()->intermediates().at(remoteNodePathPosition + 1));
        }
        return resultDone();
    }

    if (message->state() != IntermediateNodeCycleReservationResponseMessage::Accepted) {
        warning() << "Unexpected message state. Protocol error.";
        rollBack();
        return resultDone();
    }

    const auto reservedAmount = message->amountReserved();
    debug() << "Sender reserved " << reservedAmount;

    mPathStats->setNodeState(
        remoteNodePathPosition,
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
        debug() << "Total count of all participants without coordinator is "
                << mPathStats->path()->intermediates().size();

#ifdef TESTS
        mSubsystemsController->testForbidSendMessageWithFinalPathConfiguration(
            mPathStats->path()->intermediates().size());
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
    if (! contextIsValid(Message::Payments_IntermediateNodeCycleReservationRequest, false)) {
        return reject("No amount reservation request was received. Rolled back.");
    }

    const auto kMessage = popNextMessage<IntermediateNodeCycleReservationRequestMessage>();
    if (kMessage->senderAddresses.at(0) != mPreviousNode) {
        warning() << "Sender " << kMessage->senderAddresses.at(0)->fullAddress() << " is not a previous node";
        return resultContinuePreviousState();
    }
    debug() << "Coordinator payment operation from node (" << mPreviousNode->fullAddress() << ")";
    debug() << "Requested amount reservation: " << kMessage->amount();

    if (!mTrustLinesManager->trustLineContractorKeysPresent(mPreviousNodeID)) {
        warning() << "There are no contractor keys on TL";
        rollBack();
        informIntermediateNodesAboutTransactionFinish(
            mPathStats->currentIntermediateNodeAndPos().second);
        return resultDone();
    }

    const auto kIncomingAmounts = mTrustLinesManager->availableIncomingCycleAmounts(mPreviousNodeID);
    const auto kIncomingAmountWithReservations = kIncomingAmounts.first;
    const auto kIncomingAmountWithoutReservations = kIncomingAmounts.second;
    debug() << "IncomingAmountWithReservations: " << *kIncomingAmountWithReservations
            << " IncomingAmountWithoutReservations: " << *kIncomingAmountWithoutReservations;
    if (*kIncomingAmountWithReservations == TrustLine::kZeroAmount()) {
        if (*kIncomingAmountWithoutReservations == TrustLine::kZeroAmount()) {
            warning() << "can't reserve requested amount, because coordinator incoming amount "
                "event without reservations equal zero, transaction closed";
            rollBack();
            informIntermediateNodesAboutTransactionFinish(
                mPathStats->currentIntermediateNodeAndPos().second);
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
        auto reservations = mTrustLinesManager->reservationsFromContractor(mPreviousNodeID);
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

    if (0 == mIncomingAmount || ! reserveIncomingAmount(mPreviousNodeID, mIncomingAmount, 0)) {
        warning() << "No incoming amount reservation is possible. Rolled back.";
        rollBack();
        informIntermediateNodesAboutTransactionFinish(
            mPathStats->currentIntermediateNodeAndPos().second);
        return resultDone();
    }

#ifdef TESTS
    // todo : adapt SubsystemsController
//    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
//        kMessage->senderUUID,
//        mIncomingAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    debug() << "send accepted message with reserve (" << mIncomingAmount
            << ") to node " << mPreviousNode->fullAddress();
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
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

    if (! reserveIncomingAmount(mPreviousNodeID, mIncomingAmount, 0)) {
        warning() << "No incoming amount reservation is possible. Rolled back.";
        rollBack();
        informIntermediateNodesAboutTransactionFinish(
            mPathStats->currentIntermediateNodeAndPos().second);
        return resultDone();
    }

#ifdef TESTS
    // todo : adapt SubsystemsController
//    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
//        mPreviousNode,
//        mIncomingAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    debug() << "send accepted message with reserve (" << mIncomingAmount
            << ") to node " << mPreviousNode->fullAddress();
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
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
    auto senderAddress = kMessage->senderAddresses.at(0);
    debug() << "sender: " << senderAddress->fullAddress();
    if (mPaymentNodesIds.find(senderAddress->fullAddress()) == mPaymentNodesIds.end()) {
        warning() << "Sender is not participant of this transaction";
        return resultContinuePreviousState();
    }
    if (kMessage->state() == FinalAmountsConfigurationResponseMessage::Rejected) {
        return reject("Haven't reach consensus on reservation. Transaction rejected.");
    }
    debug() << "Sender confirmed final amounts";
    mParticipantsPublicKeys[mPaymentNodesIds[senderAddress->fullAddress()]] = kMessage->publicKey();

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
    mAllNodesSentConfirmationOnFinalAmountsConfiguration = true;
    if (mAllNeighborsSentFinalReservations) {
        debug() << "Begin processing participants votes.";
        return propagateVotesListAndWaitForVotingResult();
    } else {
        return resultWaitForMessageTypes(
            {Message::Payments_TransactionPublicKeyHash},
            maxNetworkDelay(1));
    }
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runFinalReservationsNeighborConfirmation()
{
    debug() << "runFinalReservationsNeighborConfirmation";
    auto kMessage = popNextMessage<TransactionPublicKeyHashMessage>();
    if (kMessage->senderAddresses.at(0) != mPreviousNode) {
        warning() << "Sender " << kMessage->senderAddresses.at(0)->fullAddress() << " is not a previous node";
        return reject("Sender invalid. Rejected");
    }

    mParticipantsPublicKeysHashes[mPreviousNode->fullAddress()] = make_pair(
        kMessage->paymentNodeID(),
        kMessage->transactionPublicKeyHash());

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (!kMessage->isReceiptContains()) {
        // coordinator should receive only 1 message from last intermediate node
        // and this message should contain receipt
        return reject("Sender send message without receipt. Rejected");
    }
    auto participantTotalIncomingReservationAmount = totalReservedIncomingAmountToNode(
        mPreviousNodeID);
    info() << "Sender also send receipt";

    auto keyChain = mKeysStore->keychain(
        mTrustLinesManager->trustLineID(mPreviousNodeID));
    auto serializedIncomingReceiptData = getSerializedReceipt(
        mContractorsManager->idOnContractorSide(mPreviousNodeID),
        mPreviousNodeID,
        participantTotalIncomingReservationAmount,
        false);
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
        mTrustLinesManager->auditNumber(mPreviousNodeID),
        mTransactionUUID,
        kMessage->publicKeyNumber(),
        participantTotalIncomingReservationAmount,
        kMessage->signature())) {
        return reject("Can't save participant receipt. Rejected.");
    }
    info() << "Sender's receipt is valid";
    mAllNeighborsSentFinalReservations = true;
    info() << "All neighbors sent theirs reservations";
    if (mAllNodesSentConfirmationOnFinalAmountsConfiguration) {
        debug() << "Begin processing participants votes.";
        return propagateVotesListAndWaitForVotingResult();
    }
    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfigurationResponse},
        maxNetworkDelay(1));
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::runVotesConsistencyCheckingStage()
{
    debug() << "runVotesConsistencyCheckingStage";
    if (! contextIsValid(Message::Payments_ParticipantVote)) {
        return reject("Coordinator didn't receive all messages with votes");
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnVoteConsistencyStage(
        mPaymentParticipants.size());
    mSubsystemsController->testThrowExceptionOnVoteConsistencyStage();
    mSubsystemsController->testTerminateProcessOnVoteConsistencyStage();
#endif

    const auto kMessage = popNextMessage<ParticipantVoteMessage>();
    auto senderAddress = kMessage->senderAddresses.at(0);
    debug () << "Participant vote message received from " << senderAddress->fullAddress();

    if (mPaymentNodesIds.find(senderAddress->fullAddress()) == mPaymentNodesIds.end()) {
        warning() << "Sender is not participant of current transaction";
        return resultContinuePreviousState();
    }

    auto participantSignature = kMessage->signature();
    auto participantPaymentID = mPaymentNodesIds[senderAddress->fullAddress()];
    auto participantPublicKey = mParticipantsPublicKeys[participantPaymentID];
    auto participantSerializedVotesData = getSerializedParticipantsVotesData(
        senderAddress);
    // todo if we store participants public keys on database, then we should use KeyChain,
    // or we can check sign directly from mParticipantsPublicKeys
    if (!participantSignature->check(
            participantSerializedVotesData.first.get(),
            participantSerializedVotesData.second,
            participantPublicKey)) {
        return reject("Participant signature is incorrect. Rolling back");
    }
    info() << "Participant signature is correct";
    mParticipantsSignatures.insert(
        make_pair(
            participantPaymentID,
            participantSignature));

    if (mParticipantsSignatures.size() + 1 == mPaymentParticipants.size()) {
        info() << "all participants sign their data";

        auto serializedOwnVotesData = getSerializedParticipantsVotesData(
            mContractorsManager->ownAddresses().at(0));
        {
            auto ioTransaction = mStorageHandler->beginTransaction();
            auto ownSignature = mKeysStore->signPaymentTransaction(
                ioTransaction,
                currentTransactionUUID(),
                serializedOwnVotesData.first,
                serializedOwnVotesData.second);
            mParticipantsSignatures.insert(
                make_pair(
                    kCoordinatorPaymentNodeID,
                    ownSignature));
        }
        debug() << "Voted +";
        mParticipantsVotesMessage = make_shared<ParticipantsVotesMessage>(
            mEquivalent,
            mContractorsManager->ownAddresses(),
            mTransactionUUID,
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
    if (path->length() < 1 || path->length() > 5) {
        throw ValueError("CycleCloserInitiatorTransaction::checkPath: "
                             "invalid paths length");
    }
    return;
    // todo : check on own node
    auto itGlobal = path->intermediates().begin();
    while (itGlobal != path->intermediates().end() - 1) {
        auto itLocal = itGlobal + 1;
        while (itLocal != path->intermediates().end() - 1) {
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
    for (SerializedPositionInPath nodePosition = 0; nodePosition <= lastInformedNodePosition; nodePosition++) {
        sendMessage<TTLProlongationResponseMessage>(
            mPathStats->path()->intermediates().at(nodePosition),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Finish);
        debug() << "send message with transaction finishing instruction to node "
                << mPathStats->path()->intermediates().at(nodePosition)->fullAddress();
    }
}

bool CycleCloserInitiatorTransaction::sendFinalPathConfiguration(
    const TrustLineAmount &finalPathAmount)
{
    debug() << "sendFinalPathConfiguration";
    mPaymentParticipants.insert(make_pair(
        kCoordinatorPaymentNodeID,
        mContractorsManager->ownAddresses().at(0)));
    mPaymentNodesIds.insert(
        make_pair(
            mContractorsManager->ownAddresses().at(0)->fullAddress(),
            kCoordinatorPaymentNodeID));
    PaymentNodeID currentNodeID = kCoordinatorPaymentNodeID + 1;
    for (const auto &intermediateNode : mPathStats->path()->intermediates()) {
        if (intermediateNode == mPaymentParticipants[kCoordinatorPaymentNodeID]) {
            // skip coordinator
            continue;
        }
        mPaymentParticipants.insert(
            make_pair(
                currentNodeID,
                intermediateNode));
        mPaymentNodesIds.insert(
            make_pair(
                intermediateNode->fullAddress(),
                currentNodeID));
        currentNodeID++;
    }
    mParticipantsPublicKeys.clear();
    for (const auto &paymentNodeIdAndAddress : mPaymentParticipants) {
        if (paymentNodeIdAndAddress.first == kCoordinatorPaymentNodeID) {
            continue;
        }
        if (paymentNodeIdAndAddress.second == mNextNode) {
            // we should send receipt to first intermediate node
            auto keyChain = mKeysStore->keychain(
                mTrustLinesManager->trustLineID(mNextNodeID));
            // coordinator should have one outgoing reservation to first intermediate node
            auto serializedOutgoingReceiptData = getSerializedReceipt(
                mNextNodeID,
                mContractorsManager->idOnContractorSide(mNextNodeID),
                finalPathAmount,
                true);
            auto ioTransaction = mStorageHandler->beginTransaction();
            auto signatureAndKeyNumber = keyChain.sign(
                ioTransaction,
                serializedOutgoingReceiptData.first,
                serializedOutgoingReceiptData.second);
            if (!keyChain.saveOutgoingPaymentReceipt(
                    ioTransaction,
                    mTrustLinesManager->auditNumber(mNextNodeID),
                    mTransactionUUID,
                    signatureAndKeyNumber.second,
                    finalPathAmount,
                    signatureAndKeyNumber.first)) {
                warning() << "Can't save outgoing receipt.";
                return false;
            }
            info() << "send message with final path amount info for node "
                   << mNextNode->fullAddress() << " with receipt";
            sendMessage<FinalPathCycleConfigurationMessage>(
                mNextNode,
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                finalPathAmount,
                mPaymentParticipants,
                signatureAndKeyNumber.second,
                signatureAndKeyNumber.first);
        } else {
            info() << "send message with final path amount info for node " << paymentNodeIdAndAddress.second->fullAddress();
            sendMessage<FinalPathCycleConfigurationMessage>(
                paymentNodeIdAndAddress.second,
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                finalPathAmount,
                mPaymentParticipants);
        }
    }
    return true;
}

TransactionResult::SharedConst CycleCloserInitiatorTransaction::approve()
{
    mCommittedAmount = totalReservedAmount(
            AmountReservation::Outgoing);
    BasePaymentTransaction::approve();
    for (const auto &paymentNodeIdAndAddress : mPaymentParticipants) {
        if (paymentNodeIdAndAddress.first == kCoordinatorPaymentNodeID) {
            continue;
        }
        sendMessage(
            paymentNodeIdAndAddress.second,
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