#include "CoordinatorPaymentTransaction.h"

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    const CreditUsageCommand::Shared command,
    bool iAmGateway,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    ResourcesManager *resourcesManager,
    PathsManager *pathsManager,
    Keystore *keystore,
    bool isPaymentTransactionsAllowedDueToObserving,
    EventsInterfaceManager *eventsInterfaceManager,
    Logger &log,
    SubsystemsController *subsystemsController):

    BasePaymentTransaction(
        BaseTransaction::CoordinatorPaymentTransaction,
        command->equivalent(),
        iAmGateway,
        contractorsManager,
        trustLines,
        storageHandler,
        topologyCacheManager,
        maxFlowCacheManager,
        resourcesManager,
        keystore,
        log,
        subsystemsController),
    mCommand(command),
    mPathsManager(pathsManager),
    mEventsInterfaceManager(eventsInterfaceManager),
    mReservationsStage(0),
    mDirectPathIsAlreadyProcessed(false),
    mCountReceiverInaccessible(0),
    mPreviousInaccessibleNodesCount(0),
    mPreviousRejectedTrustLinesCount(0),
    mRebuildingAttemptsCount(0),
    mNeighborsKeysProblem(false),
    mParticipantsKeysProblem(false),
    mIsPaymentTransactionsAllowedDueToObserving(isPaymentTransactionsAllowedDueToObserving)
{
    mStep = Stages::Coordinator_Initialization;
    mContractor = make_shared<Contractor>(command->contractorAddresses());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::run()
{
    while (true) {
        debug() << "run: stage: " << mStep;
        try {
            switch (mStep) {
                case Stages::Coordinator_Initialization:
                    return runPaymentInitializationStage();

                case Stages::Coordinator_ReceiverResourceProcessing:
                    return runPathsResourceProcessingStage();

                case Stages::Coordinator_ReceiverResponseProcessing:
                    return runReceiverResponseProcessingStage();

                case Stages::Coordinator_AmountReservation:
                    return runAmountReservationStage();

                case Stages::Coordinator_ShortPathAmountReservationResponseProcessing:
                    return runDirectAmountReservationResponseProcessingStage();

                case Stages::Common_ObservingBlockNumberProcessing:
                    return sendFinalAmountsConfigurationToAllParticipants();

                case Stages::Coordinator_FinalAmountsConfigurationConfirmation:
                    return runFinalAmountsConfigurationConfirmation();

                case Stages::Common_VotesChecking:
                    return runVotesConsistencyCheckingStage();

                default:
                    throw RuntimeError(
                            "CoordinatorPaymentTransaction::run(): "
                                    "invalid transaction step.");
            }
        } catch (CallChainBreakException &e) {
            warning() << e.what();
            // on this case we break call functions chain and prevent stack overflow
            mReservationsStage = 2;
            continue;
        } catch (Exception &e) {
            warning() << e.what();
            auto ioTransaction = mStorageHandler->beginTransaction();
            if (ioTransaction->historyStorage()->whetherOperationWasConducted(currentTransactionUUID())) {
                warning() << "Something happens wrong in method run(), but transaction was conducted";
                return resultOK();
            }
            removeAllDataFromStorageConcerningTransaction(ioTransaction);
            ioTransaction->paymentTransactionsHandler()->deleteRecord(
                mTransactionUUID);
            return reject("Something happens wrong in method run(). Transaction will be rejected");
        }
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runPaymentInitializationStage()
{
    if (!mIsPaymentTransactionsAllowedDueToObserving) {
        warning() << "It is forbid to run payment transactions due to observing";
        return resultForbiddenRunDueObserving();
    }
    if (!mSubsystemsController->isRunPaymentTransactions()) {
        debug() << "It is forbidden run payment transactions";
        return resultForbiddenRun();
    }
    debug() << "Operation initialised to the node " << mContractor->mainAddress()->fullAddress();
    debug() << "CommandUUID: " << mCommand->UUID();
    debug() << "Operation amount: " << mCommand->amount();

    if (mContractor == mContractorsManager->selfContractor()) {
        warning() << "Attempt to initialise operation against itself was prevented. Canceled.";
        return resultProtocolError();
    }

    // Check if total outgoing possibilities of this node are not smaller,
    // than total operation amount. In case if so - there is no reason to begin the operation:
    // current node would not be able to pay such an amount.
    const auto kTotalOutgoingPossibilities = *(mTrustLinesManager->totalOutgoingAmount());
    if (kTotalOutgoingPossibilities < mCommand->amount()) {
        warning() << "Total outgoing possibilities (" << kTotalOutgoingPossibilities << ") less then operation amount";
        return resultInsufficientFundsError();
    }

    mResourcesManager->requestPaths(
        currentTransactionUUID(),
        mContractor->mainAddress(),
        mEquivalent);

    mStep = Stages::Coordinator_ReceiverResourceProcessing;
    return resultWaitForResourceTypes(
        {BaseResource::Paths},
        // this delay should be greater than time of FindPathByMaxFlowTransaction running,
        // because we didn't get resources
        maxNetworkDelay(10));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runPathsResourceProcessingStage()
{
    debug() << "runPathsResourceProcessingStage";
    if (!resourceIsValid(BaseResource::Paths)) {
        return resultNoPathsError();
    }
    auto response = popNextResource<PathsResource>();
    response->pathCollection()->resetCurrentPath();
    while (response->pathCollection()->hasNextPath()) {
        auto path = response->pathCollection()->nextPath();
        info() << "path " << path->toString();
        if (isPathValid(path)) {
            addPathForFurtherProcessing(path);
        } else {
            warning() << "Invalid path: " << path->toString();
        }
    }

    // If there is no one path to the receiver - transaction can't proceed.
    if (mPathsStats.empty()) {
        warning() << "There are no paths";
        return resultNoPathsError();
    }

    debug() << "Collected paths count: " << mPathsStats.size();

    // TODO: Ensure paths shuffling

    // Sending message to the receiver note to approve the payment receiving.
    sendMessage<ReceiverInitPaymentRequestMessage>(
        mContractor->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        mCommand->amount(),
        mCommand->payload());

    mStep = Stages::Coordinator_ReceiverResponseProcessing;
    // delay 4 = 6sec for message delivery guarantee
    return resultWaitForMessageTypes(
        {Message::Payments_ReceiverInitPaymentResponse,
         Message::General_NoEquivalent},
        maxNetworkDelay(4));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runReceiverResponseProcessingStage ()
{
    if (contextIsValid(Message::General_NoEquivalent, false)) {
        warning() << "Receiver hasn't TLs on requested equivalent. Canceling.";
        return resultProtocolError();
    }
    if (!contextIsValid(Message::Payments_ReceiverInitPaymentResponse)) {
        warning() << "Receiver reservation response wasn't received. Canceling.";
        return resultNoResponseError();
    }

    const auto kMessage = popNextMessage<ReceiverInitPaymentResponseMessage>();
    if (kMessage->state() != ReceiverInitPaymentResponseMessage::Accepted) {
        info() << "Receiver rejected payment operation. Canceling.";
        return resultInsufficientFundsError();
    }

    debug() << "Receiver accepted operation. Begin reserving amounts.";
    mCurrentFreePaymentID = kCoordinatorPaymentNodeID;
    auto selfContractor = make_shared<Contractor>(mContractorsManager->ownAddresses());
    mPaymentParticipants.insert(
        make_pair(
            mCurrentFreePaymentID,
            selfContractor));
    mPaymentNodesIds.insert(
        make_pair(
            selfContractor->mainAddress()->fullAddress(),
            mCurrentFreePaymentID));
    mCurrentFreePaymentID++;
    mPaymentParticipants.insert(
        make_pair(
            mCurrentFreePaymentID,
            mContractor));
    mPaymentNodesIds.insert(
        make_pair(
            mContractor->mainAddress()->fullAddress(),
            mCurrentFreePaymentID));
    mCurrentFreePaymentID++;
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
            return runTTLTransactionResponse();
        }
        const auto kPathStats = currentAmountReservationPathStats();
        if (!kPathStats->containsIntermediateNodes()) {
            // In case if path doesn't contains intermediate nodes -
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

TransactionResult::SharedConst CoordinatorPaymentTransaction::propagateVotesListAndWaitForVotingResult()
{
    debug() << "propagateVotesListAndWaitForVotingResult. Total participants included: "
            << mParticipantsPublicKeys.size();
#ifdef DEBUG
    debug() << "Participants order is the next:";
    for (const auto &paymentNodeIdAndContractor : mPaymentParticipants) {
        debug() << paymentNodeIdAndContractor.first << " " << paymentNodeIdAndContractor.second->mainAddress()->fullAddress();
    }
#endif

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnVoteStage();
#endif

    // send message with all public keys to all participants and wait for voting results
    for (const auto &paymentNodeIdAndAddress : mPaymentParticipants) {
        if (paymentNodeIdAndAddress.first == kCoordinatorPaymentNodeID) {
            continue;
        }
        sendMessage<ParticipantsPublicKeysMessage>(
            paymentNodeIdAndAddress.second->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            mParticipantsPublicKeys);
    }

    // TODO: additional check if payment is correct

    mParticipantsSignatures.clear();

#ifdef TESTS
    mSubsystemsController->testThrowExceptionOnVoteStage();
    mSubsystemsController->testTerminateProcessOnVoteStage();
#endif

    mStep = Stages::Common_VotesChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantVote,
         Message::Payments_TTLProlongationRequest},
        maxNetworkDelay(6));
}

void CoordinatorPaymentTransaction::initAmountsReservationOnNextPath()
{
    if (mPathsStats.empty())
        throw NotFoundError(
            "CoordinatorPaymentTransaction::tryBlockAmounts: "
            "no paths are available.");

    mCurrentAmountReservingPathIdentifier = *mPathIDs.cbegin();
    debug() << "[" << mCurrentAmountReservingPathIdentifier << "] {"
            << currentAmountReservationPathStats()->path()->toString() << "}";
    mCurrentPathParticipants.clear();
}

/*
 * Tries to reserve amount on path that consists only of sender and receiver nodes.
 */
TransactionResult::SharedConst CoordinatorPaymentTransaction::tryReserveAmountDirectlyOnReceiver (
    const PathID pathID,
    PathStats *pathStats)
{
    debug() << "tryReserveAmountDirectlyOnReceiver";
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(pathStats->path()->length() == 1);
#endif

    if (mDirectPathIsAlreadyProcessed) {
        warning() << "Direct path reservation attempt occurred, but previously it was already processed. "
                << "It seems that paths collection contains direct path several times. "
                << "This one and all other similar path would be rejected. "
                << "Switching to the other path.";

        pathStats->setUnusable();
        return tryProcessNextPath();
    }
    mDirectPathIsAlreadyProcessed = true;

    debug() << "Direct path occurred (coordinator -> receiver). "
           << "Trying to reserve amount directly on the receiver side.";

    auto receiverID = mContractorsManager->contractorIDByAddress(
        mContractor->mainAddress());
    if (receiverID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Direct path wrong because receiver is not neighbor of contractor";
        pathStats->setUnusable();
        return tryProcessNextPath();
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLinesManager->trustLineOwnKeysPresent(receiverID)) {
        warning() << "There are no own keys on TL with receiver. Switching to another path.";
        pathStats->setUnusable();
        mNeighborsKeysProblem = true;
        return tryProcessNextPath();
    }

    // ToDo: implement operator < for TrustLineAmount and remove this pure conversion

    // Check if local reservation is possible.
    // If not - there is no reason to send any reservations requests.
    const auto kAvailableOutgoingAmount = mTrustLinesManager->outgoingTrustAmountConsideringReservations(receiverID);
    if (*kAvailableOutgoingAmount == TrustLine::kZeroAmount()) {
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
        receiverID,
        kReservationAmount,
        pathID)){
        warning() << "Can't reserve amount locally. Switching to another path.";

        pathStats->setUnusable();
        return tryProcessNextPath();
    }

    // Reserving on the contractor side
    pathStats->shortageMaxFlow(kReservationAmount);
    vector<pair<PathID, ConstSharedTrustLineAmount>> reservations;
    reservations.emplace_back(
        mCurrentAmountReservingPathIdentifier,
        make_shared<const TrustLineAmount>(kReservationAmount));

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToReceiverOnReservationStage();
#endif

    debug() << "Send reservations size: " << reservations.size();
    sendMessage<IntermediateNodeReservationRequestMessage>(
        receiverID,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
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
    try {
        const auto remoteAddressAndPos = pathStats->nextIntermediateNodeAndPos();
        const auto remoteAddress = remoteAddressAndPos.first;
        const auto remoteNodePositionInPath = remoteAddressAndPos.second;

        if (remoteNodePositionInPath == 0) {
            if (pathStats->isNeighborAmountReserved())
                return askNeighborToApproveFurtherNodeReservation(
                    remoteAddress,
                    pathStats);

            else
                return askNeighborToReserveAmount(
                    remoteAddress,
                    pathStats);

        } else {
            debug() << "Processing " << int(remoteNodePositionInPath)
                    << " node in path: (" << remoteAddress->fullAddress() << ").";

            const auto nextAfterRemoteNodeAddress = pathStats->path()->intermediates()[remoteNodePositionInPath + 1];
            return askRemoteNodeToApproveReservation(
                pathStats,
                remoteAddress,
                remoteNodePositionInPath,
                nextAfterRemoteNodeAddress);
        }

    } catch(NotFoundError&) {
        debug() << "No unprocessed paths are left. Requested amount can't be collected. Canceling.";
        rollBack();
        informAllNodesAboutTransactionFinish();
        return resultInsufficientFundsError();
    }
}

void CoordinatorPaymentTransaction::addPathForFurtherProcessing(
    Path::Shared path)
{
    // Preventing paths duplication
    for (const auto &identifierAndStats : mPathsStats) {
        if (identifierAndStats.second->path() == path)
            throw ConflictError("CoordinatorPaymentTransaction::addPathForFurtherProcessing: "
                                        "duplicated path occurred in the transaction.");
    }

    PathID currentPathID = 0;
    for (;;) {
        // Cycle is needed to prevent possible hashes collision.
        PathID identifier = currentPathID++;// boost::uuids::random_generator()();
        if (mPathsStats.count(identifier) == 0){
            mPathsStats[identifier] = make_unique<PathStats>(path);
            mPathsStats[identifier]->path()->addReceiver(mContractor->mainAddress());
            mPathIDs.push_back(identifier);
            return;
        }
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askNeighborToReserveAmount(
    BaseAddress::Shared neighbor,
    PathStats *path)
{
    debug() << "askNeighborToReserveAmount " << neighbor->fullAddress() ;
    auto neighborID = mContractorsManager->contractorIDByAddress(neighbor);
    if (neighborID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Contractor " << neighbor->fullAddress() << " is not a neighbor";
        throw RuntimeError(
                "CoordinatorPaymentTransaction::askNeighborToReserveAmount: "
                        "invalid first level node occurred. ");
    }
    info() << "Neighbor ID " << neighborID;
    if (! mTrustLinesManager->trustLineIsPresent(neighborID)) {
        // Internal process error.
        // No next path must be selected.
        // Transaction execution must be cancelled.

        warning() << "Invalid path occurred. Node (" << neighbor << ") is not listed in first level contractors list."
                    << " This may signal about protocol/data manipulations.";

        throw RuntimeError(
            "CoordinatorPaymentTransaction::askNeighborToReserveAmount: "
            "invalid first level TL occurred. ");
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLinesManager->trustLineOwnKeysPresent(neighborID)) {
        warning() << "There are no own keys on TL with neighbor. Switching to another path.";
        path->setUnusable();
        mNeighborsKeysProblem = true;
        throw CallChainBreakException("Break call chain for preventing call loop");
    }

    // Note: copy of shared pointer is required
    const auto kAvailableOutgoingAmount =  mTrustLinesManager->outgoingTrustAmountConsideringReservations(neighborID);
    // Note: try reserve remaining part of command amount
    const auto kRemainingAmountForProcessing =
            mCommand->amount() - totalReservedAmount(AmountReservation::Outgoing);

    const auto kReservationAmount = min(*kAvailableOutgoingAmount, kRemainingAmountForProcessing);

    if (kReservationAmount == 0) {
        debug() << "AvailableOutgoingAmount " << *kAvailableOutgoingAmount;
        debug() << "RemainingAmountForProcessing " << kRemainingAmountForProcessing;
        debug() << "No payment amount is available for. Switching to another path.";

        // todo add separated entity for rejected TL with first level
        mRejectedTrustLines.emplace_back(
            mContractorsManager->ownAddresses().at(0),
            neighbor);

        path->setUnusable();
        throw CallChainBreakException("Break call chain for preventing call loop");
    }

    if (not reserveOutgoingAmount(
        neighborID,
        kReservationAmount,
        mCurrentAmountReservingPathIdentifier)) {
            warning() << "Can't reserve amount locally. Switching to another path.";
            path->setUnusable();
            throw CallChainBreakException("Break call chain for preventing call loop");
    }

    // Try reserve amount locally.
    path->shortageMaxFlow(kReservationAmount);
    path->setNodeState(
        0,
        PathStats::NeighbourReservationRequestSent);

    vector<pair<PathID, ConstSharedTrustLineAmount>> reservations;
    reservations.emplace_back(
        mCurrentAmountReservingPathIdentifier,
        make_shared<const TrustLineAmount>(kReservationAmount));

    if (mNodesFinalAmountsConfiguration.find(neighbor->fullAddress()) != mNodesFinalAmountsConfiguration.end()) {
        // add existing neighbor reservations
        const auto kNeighborReservations = mNodesFinalAmountsConfiguration[neighbor->fullAddress()];
        reservations.insert(
            reservations.end(),
            kNeighborReservations.begin(),
            kNeighborReservations.end());
    }
    debug() << "Prepared for sending reservations size: " << reservations.size();

#ifdef TESTS
    mSubsystemsController->testForbidSendRequestToIntNodeOnReservationStage(
        neighbor,
        kReservationAmount);
#endif

    sendMessage<IntermediateNodeReservationRequestMessage>(
        neighborID,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        reservations);

    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse,
         Message::Payments_TTLProlongationRequest,
         Message::General_NoEquivalent},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askNeighborToApproveFurtherNodeReservation(
    BaseAddress::Shared neighbor,
    PathStats *path)
{
    debug() << "askNeighborToApproveFurtherNodeReservation " << neighbor->fullAddress();
    const auto kNextAfterNeighborNode = path->path()->intermediates()[1];

    // Note:
    // no check of "neighbor" node is needed here.
    // It was done on previous step.

    vector<pair<PathID, ConstSharedTrustLineAmount>> reservations;
    reservations.emplace_back(
        mCurrentAmountReservingPathIdentifier,
        make_shared<const TrustLineAmount>(path->maxFlow()));

    if (mNodesFinalAmountsConfiguration.find(kNextAfterNeighborNode->fullAddress()) !=
            mNodesFinalAmountsConfiguration.end()) {
        // add existing next after neighbor node reservations
        const auto kNeighborReservations = mNodesFinalAmountsConfiguration[kNextAfterNeighborNode->fullAddress()];

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
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        reservations,
        kNextAfterNeighborNode);

    debug() << "Further amount reservation request sent to the node (" << neighbor->fullAddress() << ") ["
            << path->maxFlow() << "]" << ", next node - (" << kNextAfterNeighborNode->fullAddress() << ")";

    path->setNodeState(
        0,
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
    if (contextIsValid(Message::General_NoEquivalent, false)) {
        warning() << "Receiver hasn't TLs on requested equivalent. Canceling.";
        // dropping reservation to first node
        dropReservationsOnPath(
                currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        return tryProcessNextPath();
    }
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        debug() << "No neighbor node response received. Switching to another path.";
        // dropping reservation to first node
        dropReservationsOnPath(
                currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);

        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mContractor->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);

        // remote node is inaccessible, we add it to offline nodes
        const auto kPathStats = currentAmountReservationPathStats();
        const auto addressAndPos = kPathStats->currentIntermediateNodeAndPos();
        mInaccessibleNodes.push_back(addressAndPos.first);
        debug() << addressAndPos.first->fullAddress() << " was added to offline nodes";

        return tryProcessNextPath();
    }

    auto message = popNextMessage<IntermediateNodeReservationResponseMessage>();
    auto neighborAddress = message->senderAddresses.at(0);
    info() << "Neighbor " << neighborAddress->fullAddress() << " send response";
    // todo: check message sender

    auto neighborID = mContractorsManager->contractorIDByAddress(neighborAddress);
    if (neighborID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Sender is not a neighbor. Continue previous state";
        return resultContinuePreviousState();
    }

    // todo : check if sender is the same node which was requested

    if (message->pathID() != mCurrentAmountReservingPathIdentifier) {
        warning() << "Neighbor send response on wrong path "
                  << message->pathID() << ". Continue previous state";
        return resultContinuePreviousState();
    }

    if (message->state() == IntermediateNodeReservationResponseMessage::Closed) {
        warning() << "Neighbor node doesn't approved reservation request";
        return reject("Desynchronization in reservation with Receiver occurred. Transaction closed.");
    }

    if (message->state() == IntermediateNodeReservationResponseMessage::Rejected) {
        warning() << "Neighbor node doesn't approved reservation request";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        mRejectedTrustLines.emplace_back(
            mContractorsManager->ownAddresses().at(0),
            neighborAddress);
        return tryProcessNextPath();
    }

    if (message->state() == IntermediateNodeReservationResponseMessage::RejectedDueContractorKeysAbsence) {
        warning() << "Neighbor node doesn't approved reservation request due to contractor keys absence";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        mRejectedTrustLines.emplace_back(
            mContractorsManager->ownAddresses().at(0),
            neighborAddress);
        mNeighborsKeysProblem = true;
        // todo maybe set mOwnKeysPresent into false and initiate KeysSharing TA
        return tryProcessNextPath();
    }

    if (message->state() != IntermediateNodeReservationResponseMessage::Accepted) {
        return reject("Unexpected message state. Protocol error. Transaction closed.");
    }

    debug() << "Neighbor approved reservation request.";
    auto path = currentAmountReservationPathStats();
    path->setNodeState(
        0, PathStats::NeighbourReservationApproved);

    if (message->amountReserved() != path->maxFlow()) {
        path->shortageMaxFlow(message->amountReserved());
        shortageReservationsOnPath(
            neighborID,
            mCurrentAmountReservingPathIdentifier,
            path->maxFlow());
    }

    // todo use return tryReserveNextIntermediateNodeAmount(path);
    return runAmountReservationStage();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processNeighborFurtherReservationResponse()
{
    debug() << "processNeighborFurtherReservationResponse";
    if (! contextIsValid(Message::Payments_CoordinatorReservationResponse)) {
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier,
            true);
        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mContractor->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);

        // remote node is inaccessible, we add it to offline nodes
        const auto kPathStats = currentAmountReservationPathStats();
        const auto addressAndPos = kPathStats->currentIntermediateNodeAndPos();
        mInaccessibleNodes.push_back(addressAndPos.first);
        debug() << addressAndPos.first->fullAddress() << " was added to offline nodes";

        debug() << "Switching to another path.";
        return tryProcessNextPath();
    }

    auto message = popNextMessage<CoordinatorReservationResponseMessage>();
    auto neighborAddress = message->senderAddresses.at(0);
    info() << "Neighbor " << neighborAddress->fullAddress() << " sent response";
    // todo: check message sender

    auto neighborID = mContractorsManager->contractorIDByAddress(neighborAddress);
    if (neighborID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Sender is not a neighbor. Continue previous state";
        return resultContinuePreviousState();
    }

    if (message->pathID() != mCurrentAmountReservingPathIdentifier) {
        warning() << "Neighbor send response on wrong path "
                  << message->pathID() << " . Continue previous state";
        return resultContinuePreviousState();
    }

    if (message->state() == CoordinatorReservationResponseMessage::Closed) {
        return reject("Desynchronization in reservation with Receiver occurred. Transaction closed.");
    }

    if (message->state() == CoordinatorReservationResponseMessage::NextNodeInaccessible) {
        warning() << "Next node after neighbor is inaccessible. Rejecting request.";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);

        // next after remote node is inaccessible, we add it to offline nodes
        const auto kPathStats = currentAmountReservationPathStats();
        const auto addressAndPos = kPathStats->currentIntermediateNodeAndPos();
        const auto nextNodeAddress = kPathStats->path()->intermediates()[addressAndPos.second + 1];
        if (nextNodeAddress == mContractor->mainAddress()) {
            mCountReceiverInaccessible++;
            if (mCountReceiverInaccessible >= kMaxReceiverInaccessible) {
                reject("Contractor is offline. Rollback.");
                return resultNoResponseError();
            }
        } else {
            mInaccessibleNodes.push_back(nextNodeAddress);
            debug() << nextNodeAddress->fullAddress() << " was added to offline nodes";
        }

        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mContractor->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);

        return tryProcessNextPath();
    }

    if (message->amountReserved() == 0 || message->state() == CoordinatorReservationResponseMessage::Rejected) {
        warning() << "Neighbor node doesn't accepted coordinator request.";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        // processed trustLine was rejected, we add it to Rejected TrustLines
        const auto kPathStats = currentAmountReservationPathStats();
        const auto neighborAddressAndPos = kPathStats->currentIntermediateNodeAndPos();
        const auto nextNeighborAddress = kPathStats->path()->intermediates()[neighborAddressAndPos.second + 1];
        mRejectedTrustLines.emplace_back(
            neighborAddressAndPos.first,
            nextNeighborAddress);
        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mContractor->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);
        return tryProcessNextPath();
    }

    if (message->state() == CoordinatorReservationResponseMessage::RejectedDueOwnKeysAbsence or
            message->state() == CoordinatorReservationResponseMessage::RejectedDueContractorKeysAbsence) {
        warning() << "Neighbor node doesn't accepted coordinator request due to keys absence";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        mRejectedTrustLines.emplace_back(
            mContractorsManager->ownAddresses().at(0),
            neighborAddress);
        mParticipantsKeysProblem = true;
        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mContractor->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);
        return tryProcessNextPath();
    }

    if (message->state() != CoordinatorReservationResponseMessage::Accepted) {
        return reject("Unexpected message state. Protocol error. Transaction closed.");
    }

    auto path = currentAmountReservationPathStats();
    path->setNodeState(
        0,
        PathStats::ReservationApproved);
    debug() << "Neighbor node accepted coordinator request. Reserved: " << message->amountReserved();
    mCurrentPathParticipants.push_back(
        make_shared<Contractor>(
            message->senderAddresses));

    if (message->amountReserved() != path->maxFlow()) {
        path->shortageMaxFlow(message->amountReserved());
        debug() << "Path max flow is now " << path->maxFlow();
        shortageReservationsOnPath(
            neighborID,
            mCurrentAmountReservingPathIdentifier,
            path->maxFlow());
    }

    if (path->isLastIntermediateNodeProcessed()) {

        const auto kTotalAmount = totalReservedAmount(
            AmountReservation::Outgoing);

        debug() << "Current path reservation finished";
        debug() << "Total collected amount by all paths: " << kTotalAmount;

        if (kTotalAmount > mCommand->amount()) {
            info() << "Total requested amount: " << mCommand->amount();
            return reject("Total collected amount is greater than requested amount. "
                              "It indicates that some of the nodes doesn't follows the protocol, "
                              "or that an error is present in protocol itself.");
        }

        // send final path amount to all intermediate nodes on path
        sendFinalPathConfiguration(
            path,
            mCurrentAmountReservingPathIdentifier,
            path->maxFlow());

        addFinalConfigurationOnPath(
            mCurrentAmountReservingPathIdentifier,
            path);

        if (kTotalAmount == mCommand->amount()){
            debug() << "Total requested amount: " << mCommand->amount() << ". Collected.";

            mStep = Common_ObservingBlockNumberProcessing;
            mResourcesManager->requestObservingBlockNumber(
                mTransactionUUID);
            return resultWaitForResourceTypes(
                {BaseResource::ObservingBlockNumber},
                maxNetworkDelay(1));
        }
        return tryProcessNextPath();
    }

    // todo use return tryReserveNextIntermediateNodeAmount(path);
    return runAmountReservationStage();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::askRemoteNodeToApproveReservation(
    PathStats* path,
    BaseAddress::Shared remoteNode,
    const byte remoteNodePosition,
    BaseAddress::Shared nextNodeAfterRemote)
{
    debug() << "askRemoteNodeToApproveReservation";
    vector<pair<PathID, ConstSharedTrustLineAmount>> reservations;
    reservations.emplace_back(
        mCurrentAmountReservingPathIdentifier,
        make_shared<const TrustLineAmount>(path->maxFlow()));

    if (mNodesFinalAmountsConfiguration.find(nextNodeAfterRemote->fullAddress()) !=
            mNodesFinalAmountsConfiguration.end()) {
        // add existing next after remote node reservations
        const auto kNeighborReservations = mNodesFinalAmountsConfiguration[nextNodeAfterRemote->fullAddress()];
        reservations.insert(
            reservations.end(),
            kNeighborReservations.begin(),
            kNeighborReservations.end());
    }

    debug() << "Prepared for sending reservations size: " << reservations.size();

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
        remoteNode,
        path->maxFlow());
#endif

    sendMessage<CoordinatorReservationRequestMessage>(
        remoteNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        reservations,
        nextNodeAfterRemote);

    path->setNodeState(
        remoteNodePosition,
        PathStats::ReservationRequestSent);

    debug() << "Further amount reservation request sent to the node (" << remoteNode->fullAddress() << ") ["
           << path->maxFlow() << ", next node - (" << nextNodeAfterRemote->fullAddress() << ")]";

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
            mCurrentAmountReservingPathIdentifier,
            true);
        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mContractor->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);
        debug() << "Switching to another path.";

        // remote node is inaccessible, we add it to offline nodes
        const auto kPathStats = currentAmountReservationPathStats();
        const auto addressAndPos = kPathStats->currentIntermediateNodeAndPos();
        mInaccessibleNodes.push_back(addressAndPos.first);
        debug() << addressAndPos.first->fullAddress() << " was added to offline nodes";

        return tryProcessNextPath();
    }

    const auto message = popNextMessage<CoordinatorReservationResponseMessage>();
    auto remoteNodeAddress = message->senderAddresses.at(0);
    info() << "Remote node " << remoteNodeAddress->fullAddress() << " sent response";
    // todo: check message sender

    if (message->pathID() != mCurrentAmountReservingPathIdentifier) {
        warning() << "Remote node sen response on wrong path " << message->pathID()
                  << " . Continue previous state";
        return resultContinuePreviousState();
    }

    if (message->state() == CoordinatorReservationResponseMessage::Closed) {
        return reject("Desynchronization in reservation with Receiver occurred. Transaction closed.");
    }

    if (message->state() == CoordinatorReservationResponseMessage::NextNodeInaccessible) {
        warning() << "Next node after remote node is inaccessible. Rejecting request.";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);

        // next after remote node is inaccessible, we add it to offline nodes
        const auto kPathStats = currentAmountReservationPathStats();
        const auto remoteNodeAddressAndPos = kPathStats->currentIntermediateNodeAndPos();
        const auto nextAfterRemoteNode = kPathStats->path()->intermediates()[remoteNodeAddressAndPos.second + 1];
        if (nextAfterRemoteNode == mContractor->mainAddress()) {
            mCountReceiverInaccessible++;
            if (mCountReceiverInaccessible >= kMaxReceiverInaccessible) {
                reject("Contractor is offline. Rollback.");
                return resultNoResponseError();
            }
        } else {
            mInaccessibleNodes.push_back(nextAfterRemoteNode);
            debug() << nextAfterRemoteNode->fullAddress() << " was added to offline nodes";
        }

        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mContractor->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);

        return tryProcessNextPath();
    }

    /*
     * Nodes scheme:
     * R - remote node;
     */

    auto path = currentAmountReservationPathStats();
    auto remoteNodeAndPos = path->currentIntermediateNodeAndPos();
    auto nextAfterRemoteNode = path->path()->intermediates()[remoteNodeAndPos.second + 1];

    if (0 == message->amountReserved() || message->state() == CoordinatorReservationResponseMessage::Rejected) {
        warning() << "Remote node rejected reservation. Switching to another path.";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        // processed trustLine was rejected, we add it to Rejected TrustLines
        mRejectedTrustLines.emplace_back(
            remoteNodeAndPos.first,
            nextAfterRemoteNode);
        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mContractor->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);

        path->setUnusable();
        path->setNodeState(
            remoteNodeAndPos.second,
            PathStats::ReservationRejected);

        return tryProcessNextPath();
    }

    if (message->state() == CoordinatorReservationResponseMessage::RejectedDueOwnKeysAbsence or
            message->state() == CoordinatorReservationResponseMessage::RejectedDueContractorKeysAbsence) {
        warning() << "Remote node doesn't accepted coordinator request due to keys absence. Switching to another path.";
        dropReservationsOnPath(
            currentAmountReservationPathStats(),
            mCurrentAmountReservingPathIdentifier);
        mRejectedTrustLines.emplace_back(
            remoteNodeAndPos.first,
            nextAfterRemoteNode);
        mParticipantsKeysProblem = true;
        // sending message to receiver that transaction continues
        sendMessage<TTLProlongationResponseMessage>(
            mContractor->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Continue);
        return tryProcessNextPath();
    }

    if (message->state() != CoordinatorReservationResponseMessage::Accepted) {
        return reject("Unexpected message state. Protocol error. Transaction closed.");
    }

    const auto reservedAmount = message->amountReserved();
    debug() << "Remote node reserved " << reservedAmount;

    mCurrentPathParticipants.push_back(
        make_shared<Contractor>(
            message->senderAddresses));
    path->setNodeState(
        remoteNodeAndPos.second,
        PathStats::ReservationApproved);

    if (reservedAmount != path->maxFlow()) {
        path->shortageMaxFlow(reservedAmount);
        auto firstIntermediateNode = path->path()->intermediates()[0];
        auto firstIntermediateNodeID = mContractorsManager->contractorIDByAddress(firstIntermediateNode);
        shortageReservationsOnPath(
            firstIntermediateNodeID,
            mCurrentAmountReservingPathIdentifier,
            path->maxFlow());
        debug() << "Path max flow is now " << path->maxFlow();
    }

    if (path->isLastIntermediateNodeProcessed()) {

        const auto kTotalAmount = totalReservedAmount(
            AmountReservation::Outgoing);

        debug() << "Current path reservation finished";
        debug() << "Total collected amount by all paths: " << kTotalAmount;

        if (kTotalAmount > mCommand->amount()){
            debug() << "Total requested amount: " << mCommand->amount();
            return reject("Total collected amount is greater than requested amount. "
                              "It indicates that some of the nodes doesn't follows the protocol, "
                              "or that an error is present in protocol itself.");
        }

        // send final path amount to all intermediate nodes on path
        sendFinalPathConfiguration(
            path,
            mCurrentAmountReservingPathIdentifier,
            path->maxFlow());

        addFinalConfigurationOnPath(
            mCurrentAmountReservingPathIdentifier,
            path);

        if (kTotalAmount == mCommand->amount()) {
            debug() << "Total requested amount: " << mCommand->amount() << ". Collected.";

            mStep = Common_ObservingBlockNumberProcessing;
            mResourcesManager->requestObservingBlockNumber(
                mTransactionUUID);
            return resultWaitForResourceTypes(
                {BaseResource::ObservingBlockNumber},
                maxNetworkDelay(1));
        }
        return tryProcessNextPath();
    }

    return tryReserveNextIntermediateNodeAmount(path);
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::tryProcessNextPath()
{
    debug() << "tryProcessNextPath";
    mCurrentPathParticipants.clear();
    try {
        switchToNextPath();
        return runAmountReservationStage();

    } catch (NotFoundError &e) {
        debug() << "No another paths are available. Try build new paths.";
        mRebuildingAttemptsCount++;
        if (mRebuildingAttemptsCount > kMaxRebuildingAttemptsCount) {
            reject("Count rebuilding attempts reaches maximal number. Canceling.");
            return resultInsufficientFundsError();
        }

        if (mInaccessibleNodes.size() != mPreviousInaccessibleNodesCount ||
                mRejectedTrustLines.size() != mPreviousRejectedTrustLinesCount) {
            auto countPathsBeforeBuilding = mPathsStats.size();
            buildPathsAgain();

            if (mPathsStats.size() > countPathsBeforeBuilding) {
                debug() << "New paths was built " << to_string(mPathsStats.size() - countPathsBeforeBuilding);
                mPreviousInaccessibleNodesCount = mInaccessibleNodes.size();
                mPreviousRejectedTrustLinesCount = mRejectedTrustLines.size();
                // in case if amount on direct paths changed, we can process it again
                mDirectPathIsAlreadyProcessed = false;
                initAmountsReservationOnNextPath();
                return runAmountReservationStage();
            }
            debug() << "New paths was not built";
        }

        reject("No another paths are available. Canceling.");
        return resultInsufficientFundsError();
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::sendFinalAmountsConfigurationToAllParticipants()
{
    debug() << "sendFinalAmountsConfigurationToAllParticipants";

    if (!resourceIsValid(BaseResource::ObservingBlockNumber)) {
        return resultUnexpectedError();
    }
    auto blockNumberResource = popNextResource<BlockNumberRecourse>();
    mMaximalClaimingBlockNumber = blockNumberResource->actualObservingBlockNumber() + kCountBlocksForClaiming;

    // check if reservation to contractor present
    auto receiverID = mContractorsManager->contractorIDByAddress(mContractor->mainAddress());
    const auto contractorNodeReservations = mReservations.find(receiverID);
    if (contractorNodeReservations != mReservations.end()) {
        if (contractorNodeReservations->second.size() > 1) {
            return reject("Coordinator has more than one reservation to contractor");
        }
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnFinalAmountClarificationStage();
#endif

    mParticipantsPublicKeys.clear();
    auto ioTransaction = mStorageHandler->beginTransaction();
    mPublicKey = mKeysStore->generateAndSaveKeyPairForPaymentTransaction(
        ioTransaction,
        currentTransactionUUID());
    mParticipantsPublicKeys.insert(
        make_pair(
            kCoordinatorPaymentNodeID,
            mPublicKey));

    for (auto const &paymentNodeIdAndContractor : mPaymentParticipants) {
        if (paymentNodeIdAndContractor.first == kCoordinatorPaymentNodeID) {
            continue;
        }
        auto participantID = mContractorsManager->contractorIDByAddress(paymentNodeIdAndContractor.second->mainAddress());
        // if coordinator has reservations with current node it also send receipt
        // if current node is not neighbor of coordinator then next condition will not work
        if (mReservations.find(participantID) != mReservations.end()) {
            auto keyChain = mKeysStore->keychain(
                mTrustLinesManager->trustLineID(participantID));
            auto outgoingReservedAmount = TrustLine::kZeroAmount();
            for (const auto &pathIDAndReservation : mReservations[participantID]) {
                // todo check if all reservations is outgoing
                outgoingReservedAmount += pathIDAndReservation.second->amount();
            }
            auto serializedOutgoingReceiptData = getSerializedReceipt(
                mContractorsManager->idOnContractorSide(participantID),
                participantID,
                outgoingReservedAmount,
                true);
            auto signatureAndKeyNumber = keyChain.sign(
                ioTransaction,
                serializedOutgoingReceiptData.first,
                serializedOutgoingReceiptData.second);
            if (!keyChain.saveOutgoingPaymentReceipt(
                    ioTransaction,
                    mTrustLinesManager->auditNumber(participantID),
                    mTransactionUUID,
                    signatureAndKeyNumber.second,
                    outgoingReservedAmount,
                    signatureAndKeyNumber.first)) {
                return reject("Can't save outgoing receipt. Rejected.");
            }
            info() << "send final amount configuration to " << paymentNodeIdAndContractor.second->mainAddress()->fullAddress()
                   << " with receipt " << outgoingReservedAmount;
            sendMessage<FinalAmountsConfigurationMessage>(
                paymentNodeIdAndContractor.second->mainAddress(),
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                mNodesFinalAmountsConfiguration[paymentNodeIdAndContractor.second->mainAddress()->fullAddress()],
                mPaymentParticipants,
                mMaximalClaimingBlockNumber,
                signatureAndKeyNumber.second,
                signatureAndKeyNumber.first,
                mPublicKey->hash());
        } else {
            info() << "send final amount configuration to " << paymentNodeIdAndContractor.second->mainAddress()->fullAddress();
            sendMessage<FinalAmountsConfigurationMessage>(
                paymentNodeIdAndContractor.second->mainAddress(),
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                mNodesFinalAmountsConfiguration[paymentNodeIdAndContractor.second->mainAddress()->fullAddress()],
                mPaymentParticipants,
                mMaximalClaimingBlockNumber);
        }
    }

    debug() << "Total count of all participants with coordinator is " << mPaymentParticipants.size();

    mStep = Coordinator_FinalAmountsConfigurationConfirmation;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfigurationResponse,
         Message::Payments_TTLProlongationRequest},
        maxNetworkDelay(6));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runFinalAmountsConfigurationConfirmation()
{
    debug() << "runFinalAmountsConfigurationConfirmation";
    if (contextIsValid(Message::Payments_TTLProlongationRequest, false)) {
        return runTTLTransactionResponse();
    }

    if (!contextIsValid(Message::Payments_FinalAmountsConfigurationResponse, false)) {
        removeAllDataFromStorageConcerningTransaction();
        return reject("Some nodes didn't confirm final amount configuration. Transaction rejected.");
    }

    auto kMessage = popNextMessage<FinalAmountsConfigurationResponseMessage>();
    auto senderAddress = kMessage->senderAddresses.at(0);
    debug() << "sender: " << senderAddress->fullAddress();
    if (mPaymentNodesIds.find(senderAddress->fullAddress()) == mPaymentNodesIds.end()) {
        warning() << "Sender is not participant of this transaction";
        return resultContinuePreviousState();
    }
    if (kMessage->state() == FinalAmountsConfigurationResponseMessage::Rejected) {
        removeAllDataFromStorageConcerningTransaction();
        return reject("Haven't reach consensus on reservation. Transaction rejected.");
    }
    debug() << "Sender confirmed final amounts";
    mParticipantsPublicKeys[mPaymentNodesIds[senderAddress->fullAddress()]] = kMessage->publicKey();
    if (mParticipantsPublicKeys.size() < mPaymentNodesIds.size()) {
        debug() << "Some nodes are still not confirmed final amounts. Waiting.";
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfigurationResponse,
             Message::Payments_TTLProlongationRequest},
            maxNetworkDelay(2));
    }

    debug() << "All nodes confirmed final configuration. Begin processing participants votes.";
    return propagateVotesListAndWaitForVotingResult();
}

PathStats* CoordinatorPaymentTransaction::currentAmountReservationPathStats()
{
    return mPathsStats[mCurrentAmountReservingPathIdentifier].get();
}

void CoordinatorPaymentTransaction::switchToNextPath()
{
    auto justProcessedPathIdentifier = mCurrentAmountReservingPathIdentifier;
    auto justProcessedPath = currentAmountReservationPathStats();
    if (! mPathIDs.empty()) {
        mPathIDs.erase(mPathIDs.cbegin());
    }

    if (mPathIDs.empty()) {
        // remove unusable path from paths scope
        if (!justProcessedPath->isValid()) {
            mPathsStats.erase(justProcessedPathIdentifier);
        }
        throw NotFoundError(
            "CoordinatorPaymentTransaction::switchToNextPath: "
                "no paths are available");
    }

    try {
        // to avoid not actual reservations in case of processing path,
        // which contains node on first position, which also is present in path,
        // processed just before, we need delay
        mCurrentAmountReservingPathIdentifier = *mPathIDs.cbegin();
        auto currentPath = currentAmountReservationPathStats();
        auto currentFirstIntermediateNode = currentPath->path()->intermediates()[0];
        auto posFirstIntermediateNodeInJustProcessedPath = justProcessedPath->path()->positionOfNode(
            currentFirstIntermediateNode);
        if (posFirstIntermediateNodeInJustProcessedPath > 0
            // if checked node was processed on previous path
            && justProcessedPath->currentIntermediateNodeAndPos().second >
               posFirstIntermediateNodeInJustProcessedPath) {
            debug() << "delay between process paths to avoid not actual reservations";
            std::this_thread::sleep_for(std::chrono::milliseconds(maxNetworkDelay(1)));
        }
        debug() << "[" << mCurrentAmountReservingPathIdentifier << "] {" << currentPath->path()->toString() << "}";
        // NotFoundError will be always in method justProcessedPath->currentIntermediateNodeAndPos()
        // on this logic it doesn't matter and we ignore it
    } catch (NotFoundError &e) {}
    // remove unusable path from paths scope
    if (!justProcessedPath->isValid()) {
        mPathsStats.erase(justProcessedPathIdentifier);
    }
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultOK()
{
    string transactionUUID = mTransactionUUID.stringUUID();
    return transactionResultFromCommand(
        mCommand->responseOK(transactionUUID));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultForbiddenRunDueObserving()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunDueObservingTransaction());
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
    if (mNeighborsKeysProblem) {
        return transactionResultFromCommand(
            mCommand->responseInsufficientFundsDueToKeysAbsent());
    }
    if (mParticipantsKeysProblem) {
        return transactionResultFromCommand(
            mCommand->responseInsufficientFundsDueToParticipantsKeysAbsent());
    }
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

const string CoordinatorPaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[CoordinatorPaymentTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::approve()
{
#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnVoteConsistencyStage(
        (uint32_t)mPaymentParticipants.size() - 1);
    // participants wait for this message 6
    mSubsystemsController->testSleepOnVoteConsistencyStage(
        maxNetworkDelay(8));
    mSubsystemsController->testThrowExceptionOnCoordinatorAfterApproveBeforeSendMessage();
#endif

    for (const auto &paymentNodeIdAndContractor : mPaymentParticipants) {
        if (paymentNodeIdAndContractor.first == kCoordinatorPaymentNodeID) {
            continue;
        }
        sendMessage(
            paymentNodeIdAndContractor.second->mainAddress(),
            mParticipantsVotesMessage);
    }

    try {
        set <PathID> actualPathsIds;
        for (const auto &nodeAndReservations : mReservations) {
            for (const auto &pathIdAndReservation : nodeAndReservations.second) {
                actualPathsIds.insert(pathIdAndReservation.first);
            }
        }
        vector<vector<BaseAddress::Shared>> paymentEventPaths;
        for (const auto &identifier : actualPathsIds) {
            const auto path = mPathsStats[identifier]->path();
            paymentEventPaths.push_back(path->intermediates());
        }

        mEventsInterfaceManager->writeEvent(
            Event::paymentEvent(
                mContractorsManager->selfContractor()->mainAddress(),
                mContractor->mainAddress(),
                paymentEventPaths,
                mTransactionUUID,
                mEquivalent));
    } catch (std::exception &e) {
        warning() << "Can't write payment event " << e.what();
    }

    mCommittedAmount = totalReservedAmount(
        AmountReservation::Outgoing);
    BasePaymentTransaction::approve();
#ifdef TESTS
    mSubsystemsController->testTerminateProcessOnCoordinatorAfterApproveBeforeSendMessage();
#endif
    BasePaymentTransaction::runThreeNodesCyclesTransactions();
    BasePaymentTransaction::runFourNodesCyclesTransactions();

    return resultOK();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::reject(
    const char *message)
{
    BasePaymentTransaction::reject(message);
    informAllNodesAboutTransactionFinish();
    return resultNoConsensusError();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runDirectAmountReservationResponseProcessingStage ()
{
    debug() << "runDirectAmountReservationResponseProcessingStage";
    auto pathStats = currentAmountReservationPathStats();
    if (not contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        debug() << "No reservation response was received from the receiver node. "
                << "Amount reservation is impossible. Switching to another path.";

        mCountReceiverInaccessible++;
        if (mCountReceiverInaccessible >= kMaxReceiverInaccessible) {
            reject("Contractor is offline. Rollback.");
            return resultNoResponseError();
        }
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
    auto receiverID = mContractorsManager->contractorIDByAddress(kMessage->senderAddresses.at(0));
    if (receiverID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Received message is not from neighbor";
        return resultContinuePreviousState();
    }

    // todo : check if sender is really receiver

    if (kMessage->state() == IntermediateNodeReservationResponseMessage::RejectedDueContractorKeysAbsence) {
        warning() << "Receiver node doesn't approved reservation request due to contractor keys absence. "
                  << "Switching to another path.";
        dropReservationsOnPath(
            pathStats,
            mCurrentAmountReservingPathIdentifier);
        mRejectedTrustLines.emplace_back(
            mContractorsManager->ownAddresses().at(0),
            kMessage->senderAddresses.at(0));
        mNeighborsKeysProblem = true;
        // todo maybe set mOwnKeysPresent into false and initiate KeysSharing TA
        mStep = Stages::Coordinator_AmountReservation;
        return tryProcessNextPath();
    }

    if (kMessage->state() != IntermediateNodeReservationResponseMessage::Accepted) {
        warning() << "Receiver node rejected reservation. "
                  << "Switching to another path.";
        dropReservationsOnPath(
            pathStats,
            mCurrentAmountReservingPathIdentifier);
        mRejectedTrustLines.emplace_back(
            mContractorsManager->ownAddresses().at(0),
            kMessage->senderAddresses.at(0));
        mStep = Stages::Coordinator_AmountReservation;
        return tryProcessNextPath();
    }

    if (kMessage->amountReserved() != pathStats->maxFlow()) {
        pathStats->shortageMaxFlow(
            kMessage->amountReserved());
        shortageReservationsOnPath(
            receiverID,
            mCurrentAmountReservingPathIdentifier,
            pathStats->maxFlow());
    }

    const auto kTotalAmount = totalReservedAmount(
        AmountReservation::Outgoing);
    debug() << "Current path reservation finished";
    debug() << "Total collected amount by all paths: " << kTotalAmount;

    if (kTotalAmount > mCommand->amount()){
        debug() << "Total requested amount: " << mCommand->amount();
        return reject("Total collected amount is greater than requested amount. "
                          "It indicates that some of the nodes doesn't follows the protocol, "
                          "or that an error is present in protocol itself.");
    }

    addFinalConfigurationOnPath(
        mCurrentAmountReservingPathIdentifier,
        pathStats);

    if (kTotalAmount == mCommand->amount()) {
        debug() << "Total requested amount: " << mCommand->amount() << ". Collected.";
        debug() << "Begin processing participants votes.";

        mStep = Common_ObservingBlockNumberProcessing;
        mResourcesManager->requestObservingBlockNumber(
            mTransactionUUID);
        return resultWaitForResourceTypes(
            {BaseResource::ObservingBlockNumber},
            maxNetworkDelay(1));
    }
    mStep = Stages::Coordinator_AmountReservation;
    return tryProcessNextPath();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runVotesConsistencyCheckingStage()
{
    debug() << "runVotesConsistencyCheckingStage";
    // Intermediate node or Receiver can send request if transaction is still alive.
    if (contextIsValid(Message::Payments_TTLProlongationRequest, false)) {
        return runTTLTransactionResponse();
    }

#ifdef TESTS
    mSubsystemsController->testThrowExceptionOnVoteConsistencyStage();
    mSubsystemsController->testTerminateProcessOnVoteConsistencyStage();
#endif

    if (! contextIsValid(Message::Payments_ParticipantVote)) {
        removeAllDataFromStorageConcerningTransaction();
        return reject("Coordinator didn't receive all messages with votes");
    }

    const auto kMessage = popNextMessage<ParticipantVoteMessage>();
    auto sender = make_shared<Contractor>(kMessage->senderAddresses);
    debug () << "Participant vote message received from " << sender->mainAddress()->fullAddress();
    if (mPaymentNodesIds.find(sender->mainAddress()->fullAddress()) == mPaymentNodesIds.end()) {
        warning() << "Sender is not participant of current transaction";
        return resultContinuePreviousState();
    }
    if (kMessage->state() == ParticipantVoteMessage::Rejected) {
        removeAllDataFromStorageConcerningTransaction();
        return reject("Participant rejected voting. Rolling back");
    }
    auto participantSignature = kMessage->signature();
    auto participantPaymentID = mPaymentNodesIds[sender->mainAddress()->fullAddress()];
    auto participantPublicKey = mParticipantsPublicKeys[participantPaymentID];
    auto participantSerializedVotesData = getSerializedParticipantsVotesData(
        sender);
    // todo if we store participants public keys on database, then we should use KeyChain,
    // or we can check sign directly from mParticipantsPublicKeys
    if (!participantSignature->check(
            participantSerializedVotesData.first.get(),
            participantSerializedVotesData.second,
            participantPublicKey)) {
        removeAllDataFromStorageConcerningTransaction();
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
            mContractorsManager->selfContractor());
        {
            auto ioTransaction = mStorageHandler->beginTransaction();
            auto ownSign = mKeysStore->signPaymentTransaction(
                ioTransaction,
                currentTransactionUUID(),
                serializedOwnVotesData.first,
                serializedOwnVotesData.second);
            mParticipantsSignatures.insert(
                make_pair(
                    kCoordinatorPaymentNodeID,
                    ownSign));

            ioTransaction->paymentTransactionsHandler()->saveRecord(
                mTransactionUUID,
                mMaximalClaimingBlockNumber);
        }
        debug() << "Voted +";
        const auto ownAddresses = mContractorsManager->ownAddresses();
        mParticipantsVotesMessage = make_shared<ParticipantsVotesMessage>(
            mEquivalent,
            ownAddresses,
            mTransactionUUID,
            mParticipantsSignatures);
        return approve();
    }

    info() << "Not all participants send theirs signs";
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantVote,
         Message::Payments_TTLProlongationRequest},
        maxNetworkDelay(3));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::runTTLTransactionResponse()
{
    debug() << "runTTLTransactionResponse";
    auto kMessage = popNextMessage<TTLProlongationRequestMessage>();
    auto senderAddress = kMessage->senderAddresses.at(0);
    info() << "sender " << senderAddress->fullAddress();
    if (mPaymentParticipants.empty()) {
        // reservation stage
        if (senderAddress == mContractor->mainAddress()) {
            sendMessage<TTLProlongationResponseMessage>(
                senderAddress,
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                TTLProlongationResponseMessage::Continue);
            debug() << "Send clarifying message that transactions is alive";
        }
        else if (mNodesFinalAmountsConfiguration.find(senderAddress->fullAddress()) !=
                mNodesFinalAmountsConfiguration.end()) {
            // coordinator has configuration for requested node
            sendMessage<TTLProlongationResponseMessage>(
                senderAddress,
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                TTLProlongationResponseMessage::Continue);
            debug() << "Send clarifying message that transactions is alive";
        } else {
            sendMessage<TTLProlongationResponseMessage>(
                senderAddress,
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                TTLProlongationResponseMessage::Finish);
            debug() << "Send transaction finishing message";
        }
    } else {
        // voting stage
        if (mPaymentNodesIds.find(senderAddress->fullAddress()) != mPaymentNodesIds.end()) {
            sendMessage<TTLProlongationResponseMessage>(
                senderAddress,
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                TTLProlongationResponseMessage::Continue);
            debug() << "Send clarifying message that transactions is alive";
        } else {
            sendMessage<TTLProlongationResponseMessage>(
                senderAddress,
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                TTLProlongationResponseMessage::Finish);
            info() << "Sender is not a member of this transaction. Continue previous state";
            debug() << "Send transaction finishing message";
        }
    }
    return resultContinuePreviousState();
}

bool CoordinatorPaymentTransaction::isPathValid(
    Path::Shared path) const
{
    if (path->length() == 0) {
        return true;
    }
    if (path->length() > kMaxPathLength - 2) {
        throw ValueError("CoordinatorPaymentTransaction::checkPath: "
                             "invalid paths length " + to_string(path->length()));
    }
    auto currentNodeMainAddress = mContractorsManager->selfContractor()->mainAddress();
    for (uint32_t idxGlobal = 0; idxGlobal < path->intermediates().size() - 1; idxGlobal++) {
        auto globalNodeAddress = path->intermediates().at(idxGlobal);
        if (currentNodeMainAddress == globalNodeAddress) {
            warning() << "Paths contains current node several times";
            return false;
        }
        if (mContractor->mainAddress() == globalNodeAddress) {
            warning() << "Paths contains receiver node several times";
            return false;
        }
        for (uint32_t idxLocal = idxGlobal + 1; idxLocal < path->intermediates().size(); idxLocal++) {
            if (globalNodeAddress == path->intermediates().at(idxLocal)) {
                warning() << "Paths contains repeated nodes " << globalNodeAddress->fullAddress();
                return false;
            }
        }
    }
    if (currentNodeMainAddress == path->intermediates().at(path->intermediates().size() - 1)) {
        warning() << "Paths contains current node several times";
        return false;
    }
    if (mContractor->mainAddress() == path->intermediates().at(path->intermediates().size() - 1)) {
        warning() << "Paths contains receiver node several times";
        return false;
    }
    return true;
}

void CoordinatorPaymentTransaction::addFinalConfigurationOnPath(
    PathID pathID,
    PathStats* pathStats)
{
    debug() << "Add final configuration on path " << pathID;
    for (const auto &contractor : mCurrentPathParticipants) {
        bool participantIncluded = false;
        for (const auto &paymentParticipant : mPaymentParticipants) {
            if (contractor == paymentParticipant.second) {
                participantIncluded = true;
                break;
            }
        }
        if (!participantIncluded) {
            mPaymentParticipants.insert(
                make_pair(
                    mCurrentFreePaymentID,
                    contractor));
            mPaymentNodesIds.insert(
                make_pair(
                    contractor->mainAddress()->fullAddress(),
                    mCurrentFreePaymentID));
            mCurrentFreePaymentID++;
        }
    }

    auto pathIDAndAmount = make_pair(
        pathID,
        make_shared<const TrustLineAmount>(
            pathStats->maxFlow()));

    // add final path configuration for all intermediate nodes
    for (const auto &contractor : mCurrentPathParticipants) {
        auto nodeKey = contractor->mainAddress()->fullAddress();
        if (mNodesFinalAmountsConfiguration.find(nodeKey) == mNodesFinalAmountsConfiguration.end()) {
            vector<pair<PathID, ConstSharedTrustLineAmount>> newVector;
            newVector.push_back(pathIDAndAmount);
            mNodesFinalAmountsConfiguration.insert(
                make_pair(
                    nodeKey,
                    newVector));
        } else {
            mNodesFinalAmountsConfiguration[nodeKey].push_back(
                pathIDAndAmount);
        }
    }
    mNodesFinalAmountsConfiguration[mContractor->mainAddress()->fullAddress()].push_back(
        pathIDAndAmount);
}

void CoordinatorPaymentTransaction::shortageReservationsOnPath(
    ContractorID neighborID,
    const PathID pathID,
    const TrustLineAmount &amount)
{
    debug() << "shortageReservationsOnPath";
    auto nodeReservations = mReservations[neighborID];
    for (const auto &pathIDAndReservation : nodeReservations) {
        if (pathIDAndReservation.first == pathID) {
            shortageReservation(
                neighborID,
                pathIDAndReservation.second,
                amount,
                pathID);
            // coordinator has only one reservation on each path
            break;
        }
    }
}

void CoordinatorPaymentTransaction::dropReservationsOnPath(
    PathStats *pathStats,
    PathID pathID,
    bool sendToLastProcessedNode)
{
    debug() << "dropReservationsOnPath";
    pathStats->setUnusable();

    auto firstIntermediateNode = pathStats->path()->intermediates()[0];
    auto firstIntermediateNodeID = mContractorsManager->contractorIDByAddress(firstIntermediateNode);
    auto nodeReservations = mReservations.find(firstIntermediateNodeID);
    auto itPathIDAndReservation = nodeReservations->second.begin();
    while (itPathIDAndReservation != nodeReservations->second.end()) {
        if (itPathIDAndReservation->first == pathID) {
            debug() << "Dropping reservation: [ => ] " << itPathIDAndReservation->second->amount()
                    << " for (" << firstIntermediateNode->fullAddress() << ") [" << pathID << "]";
            mTrustLinesManager->dropAmountReservation(
                firstIntermediateNodeID,
                itPathIDAndReservation->second);

            itPathIDAndReservation = nodeReservations->second.erase(itPathIDAndReservation);
            // coordinator has only one reservation on each path
            break;
        } else {
            itPathIDAndReservation++;
        }
    }
    if (nodeReservations->second.empty()) {
        mReservations.erase(firstIntermediateNodeID);
    }

    // send message with dropping reservation instruction to all intermediate nodes because this path is unusable
    if (pathStats->path()->length() == 1) {
        return;
    }
    const auto lastProcessedNodeAndPos = pathStats->currentIntermediateNodeAndPos();
    const auto lastProcessedNode = lastProcessedNodeAndPos.first;
    for (const auto &intermediateNode : pathStats->path()->intermediates()) {
        if (!sendToLastProcessedNode && intermediateNode == lastProcessedNode) {
            break;
        }
        debug() << "send message with drop reservation info for node " << intermediateNode->fullAddress();
        sendMessage<FinalPathConfigurationMessage>(
            intermediateNode,
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            pathID,
            TrustLine::kZeroAmount());
        if (sendToLastProcessedNode && intermediateNode == lastProcessedNode) {
            break;
        }
    }
}

void CoordinatorPaymentTransaction::sendFinalPathConfiguration(
    PathStats* pathStats,
    PathID pathID,
    const TrustLineAmount &finalPathAmount)
{
    debug() << "sendFinalPathConfiguration";
#ifdef TESTS
    mSubsystemsController->testForbidSendMessageWithFinalPathConfiguration(
        (uint32_t)pathStats->path()->intermediates().size() - 1);
#endif
    for (const auto &intermediateNode : pathStats->path()->intermediates()) {
        if (intermediateNode == mContractor->mainAddress()) {
            continue;
        }
        debug() << "send message with final path amount info for node " << intermediateNode->fullAddress();
        sendMessage<FinalPathConfigurationMessage>(
            intermediateNode,
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            pathID,
            finalPathAmount);
    }
}

void CoordinatorPaymentTransaction::informAllNodesAboutTransactionFinish()
{
    debug() << "informAllNodesAboutTransactionFinish";
    for (auto const &paymentNodeIdAndContractor : mPaymentParticipants) {
        if (paymentNodeIdAndContractor.first == kCoordinatorPaymentNodeID) {
            continue;
        }
        sendMessage<TTLProlongationResponseMessage>(
            paymentNodeIdAndContractor.second->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            TTLProlongationResponseMessage::Finish);
        debug() << "Send transaction finishing message to participant " << paymentNodeIdAndContractor.first;
    }
}

void CoordinatorPaymentTransaction::buildPathsAgain()
{
    debug() << "buildPathsAgain";
    auto startTime = utc_now();
    for (auto const &pathIDAndPathStats : mPathsStats) {
        auto const pathStats = pathIDAndPathStats.second.get();

        mPathsManager->addUsedAmountFromInitiator(
            pathStats->path()->intermediates().at(0),
            pathStats->maxFlow());
        for (SerializedPositionInPath idx = 0; idx < pathStats->path()->intermediates().size() - 1; idx++) {
            mPathsManager->addUsedAmount(
                pathStats->path()->intermediates().at(idx),
                pathStats->path()->intermediates().at(idx + 1),
                pathStats->maxFlow());
        }
    }
    for (const auto &rejectedTrustLine : mRejectedTrustLines) {
        mPathsManager->makeTrustLineFullyUsed(
            rejectedTrustLine.first,
            rejectedTrustLine.second);
    }
    mPathsManager->reBuildPaths(
        mContractor->mainAddress(),
        mInaccessibleNodes);
    mPathsManager->pathCollection()->resetCurrentPath();
    while (mPathsManager->pathCollection()->hasNextPath()) {
        auto path = mPathsManager->pathCollection()->nextPath();
        if (isPathValid(path)) {
            addPathForFurtherProcessing(path);
        }
    }
    mPathsManager->clearPathsCollection();
    debug() << "buildPathsAgain method time: " << utc_now() - startTime;
}

void CoordinatorPaymentTransaction::savePaymentOperationIntoHistory(
    IOTransaction::Shared ioTransaction)
{
    debug() << "savePaymentOperationIntoHistory";
    ioTransaction->historyStorage()->savePaymentRecord(
        make_shared<PaymentRecord>(
            currentTransactionUUID(),
            PaymentRecord::OutgoingPaymentType,
            mContractor,
            mCommittedAmount,
            *mTrustLinesManager->totalBalance().get(),
            mOutgoingTransfers,
            mIncomingTransfers,
            mCommand->UUID(),
            mCommand->payload()),
        mEquivalent);
    debug() << "Operation saved";
}

bool CoordinatorPaymentTransaction::checkReservationsDirections() const
{
    debug() << "checkReservationsDirections";
    for (const auto &nodeAndReservations : mReservations) {
        for (const auto &pathIDAndReservation : nodeAndReservations.second) {
            if (pathIDAndReservation.second->direction() != AmountReservation::Outgoing) {
                return false;
            }
        }
    }
    debug() << "All reservations directions are correct";
    return true;
}

const CommandUUID& CoordinatorPaymentTransaction::commandUUID() const
{
    return mCommand->UUID();
}
