#include "IntermediateNodePaymentTransaction.h"


IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    IntermediateNodeReservationRequestMessage::ConstShared message,
    bool iAmGateway,
    ContractorsManager *contractorsManager,
    TrustLinesManager* trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    ResourcesManager *resourcesManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BasePaymentTransaction(
        BaseTransaction::IntermediateNodePaymentTransaction,
        message->transactionUUID(),
        message->equivalent(),
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
    mMessage(message)
{
    mStep = Stages::IntermediateNode_PreviousNeighborRequestProcessing;
    mCoordinator = nullptr;
}

IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    BytesShared buffer,
    bool iAmGateway,
    ContractorsManager *contractorsManager,
    TrustLinesManager* trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    ResourcesManager *resourcesManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BasePaymentTransaction(
        buffer,
        iAmGateway,
        contractorsManager,
        trustLines,
        storageHandler,
        topologyCacheManager,
        maxFlowCacheManager,
        resourcesManager,
        keystore,
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

            case Stages::Coordinator_FinalAmountsConfigurationConfirmation:
                return runFinalAmountsConfigurationConfirmation();

            case Stages::Common_ObservingBlockNumberProcessing:
                return runCheckObservingBlockNumber();

            case Stages::Common_Voting:
                return runVotesCheckingStageWithCoordinatorClarification();

            case Stages::Common_VotesChecking:
                return runVotesConsistencyCheckingStage();

            case Stages::Common_Recovery:
                return runVotesRecoveryParentStage();

            case Stages::Common_Observing:
                return runObservingResultStage();

            case Stages::Common_ObservingReject:
                return runObservingRejectTransaction();

            case Stages::Common_Uncertain: {
                info() << "Uncertain stage";
                return resultDone();
            }

            default:
                throw RuntimeError(
                    "IntermediateNodePaymentTransaction::run: "
                        "unexpected stage occurred.");
        }
    } catch (Exception &e) {
        warning() << e.what();
        return recover("Something happens wrong in method run(). Transaction will be recovered");
    }
}


TransactionResult::SharedConst IntermediateNodePaymentTransaction::runPreviousNeighborRequestProcessingStage()
{
    debug() << "runPreviousNeighborRequestProcessingStage";
    const auto kNeighbor = mMessage->senderAddresses.at(0);
    debug() << "Init. intermediate payment operation from node (" << kNeighbor->fullAddress() << ")";

    if (mMessage->finalAmountsConfiguration().empty()) {
        warning() << "Not received reservation";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            0,                  // 0, because we don't know pathID
            ResponseMessage::Closed);
    }

    const auto kReservation = mMessage->finalAmountsConfiguration()[0];
    debug() << "Requested amount reservation: " << *kReservation.second.get() << " on path " << kReservation.first;
    debug() << "Received reservations size: " << mMessage->finalAmountsConfiguration().size();

    auto senderID = mContractorsManager->contractorIDByAddress(kNeighbor);
    if (senderID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Previous node is not a neighbor";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Rejected);
    }
    debug() << "Sender ID " << senderID;

    if (!mTrustLinesManager->trustLineIsPresent(senderID)) {
        warning() << "Path is not valid: there is no TL with previous node. Rejected.";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Rejected);
    }

    if (!mTrustLinesManager->trustLineIsActive(senderID)) {
        warning() << "Path is not valid: TL with previous node is not active. Rejected.";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Rejected);
    }

    // update local reservations during amounts from coordinator
    if (!updateReservations(vector<pair<PathID, ConstSharedTrustLineAmount>>(
        mMessage->finalAmountsConfiguration().begin() + 1,
        mMessage->finalAmountsConfiguration().end()))) {
        warning() << "Previous node send path configuration, which is absent on current node";
        // next loop is only logger info
        for (const auto &reservation : mMessage->finalAmountsConfiguration()) {
            debug() << "path: " << reservation.first << " amount: " << *reservation.second.get();
        }
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Closed);
    }
    debug() << "All reservations were updated";

    if (!mTrustLinesManager->trustLineContractorKeysPresent(senderID)) {
        warning() << "There are no contractor keys on TL";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::RejectedDueContractorKeysAbsence);
    }

    const auto kIncomingAmount = mTrustLinesManager->incomingTrustAmountConsideringReservations(senderID);
    TrustLineAmount kReservationAmount =
            min(*kReservation.second.get(), *kIncomingAmount);

#ifdef TESTS
    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
        kNeighbor,
        kReservationAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    if (0 == kReservationAmount || ! reserveIncomingAmount(senderID, kReservationAmount, kReservation.first)) {
        warning() << "No amount reservation is possible.";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Rejected);
    }

    debug() << "reserve locally " << kReservationAmount << " to node "
            << kNeighbor->fullAddress() << " on path " << kReservation.first;
    mLastReservedAmount = kReservationAmount;
    mLastProcessedPath = kReservation.first;
    sendMessage<IntermediateNodeReservationResponseMessage>(
        kNeighbor,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        kReservation.first,
        ResponseMessage::Accepted,
        kReservationAmount);

    mStep = Stages::IntermediateNode_CoordinatorRequestProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationRequest,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(4));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runCoordinatorRequestProcessingStage()
{
    debug() << "runCoordinatorRequestProcessingStage";

    if (!contextIsValid(Message::Payments_CoordinatorReservationRequest, false)) {
        warning() << "No coordinator request received.";
        return runReservationProlongationStage();
    }

    debug() << "Coordinator further reservation request received.";

#ifdef TESTS
    mSubsystemsController->testThrowExceptionOnCoordinatorRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnCoordinatorRequestProcessingStage();
#endif
    // TODO: add check for previous nodes amount reservation

    const auto kMessage = popNextMessage<CoordinatorReservationRequestMessage>();
    mCoordinator = make_shared<Contractor>(kMessage->senderAddresses);
    // todo : on this stage node should know coordinator and check sender
    const auto kNextNode = kMessage->nextNodeInPath();
    if (kMessage->finalAmountsConfiguration().empty()) {
        warning() << "Not received reservation";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseMessage::Closed);
    }

    const auto kReservation = kMessage->finalAmountsConfiguration()[0];
    mLastProcessedPath = kReservation.first;
    // TODO : add check for mLastProcessedPath

    debug() << "requested reservation amount is " << *kReservation.second.get() << " on path " << kReservation.first;
    debug() << "Next node is " << kNextNode->fullAddress();
    debug() << "Received reservations size: " << kMessage->finalAmountsConfiguration().size();
    auto nextNodeID = mContractorsManager->contractorIDByAddress(kNextNode);
    if (nextNodeID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Next node is not neighbor";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseMessage::Rejected);
    }
    info() << "Next node ID " << nextNodeID;
    if (!mTrustLinesManager->trustLineIsPresent(nextNodeID)) {
        warning() << "Path is not valid: there is no TL with next node. Rolled back.";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseMessage::Rejected);
    }

    if (!mTrustLinesManager->trustLineIsActive(nextNodeID)) {
        warning() << "Path is not valid: TL with next node is not active. Rolled back.";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseMessage::Rejected);
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLinesManager->trustLineOwnKeysPresent(nextNodeID)) {
        warning() << "There are no own keys on TL";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseMessage::RejectedDueOwnKeysAbsence);
    }

    // Note: copy of shared pointer is required
    const auto kOutgoingAmount = mTrustLinesManager->outgoingTrustAmountConsideringReservations(nextNodeID);
    debug() << "available outgoing amount to next node is " << *kOutgoingAmount;
    TrustLineAmount reservationAmount = min(
        *kReservation.second.get(),
        *kOutgoingAmount);

    if (0 == reservationAmount || ! reserveOutgoingAmount(nextNodeID, reservationAmount, kReservation.first)) {
        warning() << "No amount reservation is possible. Rolled back.";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseMessage::Rejected);
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendRequestToIntNodeOnReservationStage(
        kNextNode,
        reservationAmount);
#endif

    debug() << "Reserve locally " << reservationAmount << " to node "
            << kNextNode->fullAddress() << " on path " << kReservation.first;
    mLastReservedAmount = reservationAmount;

    // build reservation configuration for next node;
    // CoordinatorReservationRequestMessage contains configuration for next node
    vector<pair<PathID, ConstSharedTrustLineAmount>> reservations;
    reservations.emplace_back(
        kReservation.first,
        make_shared<const TrustLineAmount>(reservationAmount));

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
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        reservations);

    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_FinalPathConfiguration,
         Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse,
         Message::General_NoEquivalent},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runNextNeighborResponseProcessingStage()
{
    debug() << "runNextNeighborResponseProcessingStage";
    if (mContext.empty()) {
        warning() << "No amount reservation response received. Rolled back.";
        return sendErrorMessageOnNextNodeResponse(
            ResponseMessage::NextNodeInaccessible);
    }

    if (contextIsValid(Message::General_NoEquivalent, false)) {
        warning() << "Neighbor hasn't TLs on requested equivalent. Rolled back.";
        return sendErrorMessageOnNextNodeResponse(
            ResponseMessage::Rejected);
    }

    if (!contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        return runReservationProlongationStage();
    }

    const auto kMessage = popNextMessage<IntermediateNodeReservationResponseMessage>();
    auto nextNodeAddress = kMessage->senderAddresses.at(0);
    info() << "Next node " << nextNodeAddress->fullAddress() << " sent response";
    // todo : check sender node

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
        nextNodeAddress,
        kMessage->amountReserved());
    mSubsystemsController->testThrowExceptionOnNextNeighborResponseProcessingStage();
    mSubsystemsController->testTerminateProcessOnNextNeighborResponseProcessingStage();
#endif

    if (kMessage->pathID() != mLastProcessedPath) {
        warning() << "Next node sent response on wrong path " << kMessage->pathID()
                  << " . Continue previous state";
        return resultContinuePreviousState();
    }

    if (kMessage->state() == IntermediateNodeReservationResponseMessage::Closed) {
        warning() << "Amount reservation rejected with further transaction closing by the Receiver node.";
        // Receiver reject reservation and Coordinator should close transaction
        return sendErrorMessageOnNextNodeResponse(
            ResponseMessage::Closed);
    }

    if (kMessage->state() == IntermediateNodeReservationResponseMessage::Rejected){
        warning() << "Amount reservation rejected by the neighbor node.";
        return sendErrorMessageOnNextNodeResponse(
            ResponseMessage::Rejected);
    }

    if (kMessage->state() == IntermediateNodeReservationResponseMessage::RejectedDueContractorKeysAbsence){
        warning() << "Neighbor node doesn't approved reservation request due to contractor keys absence";
        // todo maybe set mOwnKeysPresent into false and initiate KeysSharing TA
        return sendErrorMessageOnNextNodeResponse(
            ResponseMessage::RejectedDueContractorKeysAbsence);
    }

    debug() << "Next node accepted amount reservation.";
    mLastReservedAmount = kMessage->amountReserved();

    shortageReservationsOnPath(
        kMessage->pathID(),
        kMessage->amountReserved());

#ifdef TESTS
    // coordinator wait for this message maxNetworkDelay(4)
    // we try get max delay with normal working algorithm on this stage
    mSubsystemsController->testSleepOnNextNeighborResponseProcessingStage(
        maxNetworkDelay(3) - 500);
#endif

    sendMessage<CoordinatorReservationResponseMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        kMessage->pathID(),
        ResponseMessage::Accepted,
        mLastReservedAmount);

    mStep = Stages::Common_FinalPathConfigurationChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalPathConfiguration,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runFinalPathConfigurationProcessingStage()
{
    // receive final amount on current path
    debug() << "runFinalPathConfigurationProcessingStage";

    if (!contextIsValid(Message::Payments_FinalPathConfiguration, false)) {
        warning() << "No final paths configuration was received from the coordinator.";
        return runReservationProlongationStage();
    }

    const auto kMessage = popNextMessage<FinalPathConfigurationMessage>();

    // TODO: check if message was really received from the coordinator;

    debug() << "Final payment path configuration received";

    // path was cancelled, drop all reservations belong it
    if (kMessage->amount() == 0) {
        rollBack(kMessage->pathID());
        // if no reservations close transaction
        if (mReservations.empty()) {
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
         Message::Payments_TransactionPublicKeyHash,
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

    if (contextIsValid(Message::Payments_FinalPathConfiguration, false)) {
        return runFinalPathConfigurationProcessingStage();
    }

    if (contextIsValid(Message::Payments_TransactionPublicKeyHash, false)) {
        mStep = Coordinator_FinalAmountsConfigurationConfirmation;
        return runFinalReservationsNeighborConfirmation();
    }

    if (contextIsValid(Message::Payments_FinalAmountsConfiguration, false)) {
        mTTLRequestWasSend = false;
        mStep = Coordinator_FinalAmountsConfigurationConfirmation;
        return runFinalReservationsCoordinatorConfirmation();
    }

    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
        if (kMessage->state() == TTLProlongationResponseMessage::Continue) {
            // transactions is still alive and we continue waiting for messages
            info() << "Transactions is still alive. Continue waiting for messages";
            if (utc_now() - mTimeStarted > kMaxTransactionDuration()) {
                return reject("Transaction duration time has expired. Rolling back");
            }
            // we don't wait for Payments_FinalPathConfiguration message,
            // because it isn't critical for us
            return resultWaitForMessageTypes(
                {Message::Payments_TTLProlongationResponse,
                 Message::Payments_FinalPathConfiguration,
                 Message::Payments_FinalAmountsConfiguration,
                 Message::Payments_TransactionPublicKeyHash,
                 Message::Payments_IntermediateNodeReservationRequest},
                maxNetworkDelay((kMaxPathLength - 2) * 4));
        }
        return reject("Coordinator send response with transaction finish state. Rolling Back");
    }

    if (mTTLRequestWasSend) {
        return reject("No TTL response message received. Transaction will be closed. Rolling Back");
    }

    if (mCoordinator == nullptr) {
        return reject("Node doesn't know coordinator yet. Transaction will be closed. Rolling Back");
    }
    debug() << "Send TTLTransaction message to coordinator " << mCoordinator->mainAddress()->fullAddress();
    sendMessage<TTLProlongationRequestMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID());
    mTTLRequestWasSend = true;
    // we don't wait for Payments_FinalPathConfiguration message,
    // because it isn't critical for us
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_FinalPathConfiguration,
         Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runFinalAmountsConfigurationConfirmation()
{
    debug() << "runFinalAmountsConfigurationConfirmation";
    if (contextIsValid(Message::Payments_FinalAmountsConfiguration, false)) {
        return runFinalReservationsCoordinatorConfirmation();
    }

    if (contextIsValid(Message::Payments_TransactionPublicKeyHash, false)) {
        return runFinalReservationsNeighborConfirmation();
    }

    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        debug() << "Receive TTL prolongation message";
        const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
        if (kMessage->state() == TTLProlongationResponseMessage::Continue) {
            info() << "Transactions is still alive. Continue waiting for messages";
            if (utc_now() - mTimeStarted > kMaxTransactionDuration()) {
                return reject("Transaction duration time has expired. Rolling back");
            }
            return resultWaitForMessageTypes(
                {Message::Payments_TTLProlongationResponse,
                 Message::Payments_FinalAmountsConfiguration,
                 Message::Payments_TransactionPublicKeyHash},
                maxNetworkDelay((kMaxPathLength - 1) * 4));
        }
        removeAllDataFromStorageConcerningTransaction();
        return reject("Coordinator send TTL message with transaction finish state. Rolling Back");
    }

    if (mTTLRequestWasSend) {
        removeAllDataFromStorageConcerningTransaction();
        return reject("No all final amounts configuration received. Transaction will be closed. Rolling Back");
    }

    debug() << "Send TTLTransaction message to coordinator " << mCoordinator->mainAddress()->fullAddress();
    sendMessage<TTLProlongationRequestMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID());

    mTTLRequestWasSend = true;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runFinalReservationsCoordinatorConfirmation()
{
    // receive final configuration on all paths
    debug() << "runFinalReservationsCoordinatorConfirmation";

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnFinalAmountClarificationStage();
#endif

    auto kMessage = popNextMessage<FinalAmountsConfigurationMessage>();
    auto coordinatorAddress = kMessage->senderAddresses.at(0);

    mMaximalClaimingBlockNumber = kMessage->maximalClaimingBlockNumber();
    debug() << "maximal claiming block number: " << mMaximalClaimingBlockNumber;

    // todo : check coordinator
    if (!updateReservations(
        kMessage->finalAmountsConfiguration())) {
        sendErrorMessageOnFinalAmountsConfiguration();
        removeAllDataFromStorageConcerningTransaction();
        return reject("There are some final amounts, reservations for which are absent. Rejected");
    }

#ifdef TESTS
    // coordinator wait for this message maxNetworkDelay(6)
    mSubsystemsController->testSleepOnFinalAmountClarificationStage(
        maxNetworkDelay(8));
#endif

    info() << "All reservations was updated";

    if (!checkReservationsDirections()) {
        removeAllDataFromStorageConcerningTransaction();
        return reject("Reservations on node are invalid");
    }
    info() << "All reservations are correct";

    mPaymentParticipants = kMessage->paymentParticipants();
    // todo check if current node is present in mPaymentParticipants
    for (const auto &paymentParticipant : mPaymentParticipants) {
        mPaymentNodesIds.insert(
            make_pair(
                paymentParticipant.second->mainAddress()->fullAddress(),
                paymentParticipant.first));
    }
    if (!checkAllNeighborsWithReservationsAreInFinalParticipantsList()) {
        removeAllDataFromStorageConcerningTransaction();
        sendErrorMessageOnFinalAmountsConfiguration();
        return reject("Node has reservation with participant, which not included in mPaymentNodesIds. Rejected");
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (kMessage->isReceiptContains()) {
        info() << "Coordinator also send receipt";
        auto coordinatorID = mContractorsManager->contractorIDByAddress(coordinatorAddress);
        if (coordinatorID == ContractorsManager::kNotFoundContractorID) {
            removeAllDataFromStorageConcerningTransaction(ioTransaction);
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Coordinator is not a neighbor. Rejected");
        }
        auto coordinatorTotalIncomingReservationAmount = totalReservedIncomingAmountToNode(
            coordinatorID);
        auto keyChain = mKeysStore->keychain(
            mTrustLinesManager->trustLineID(coordinatorID));
        auto serializedIncomingReceiptData = getSerializedReceipt(
            coordinatorID,
            mContractorsManager->idOnContractorSide(coordinatorID),
            coordinatorTotalIncomingReservationAmount,
            false);
        if (!keyChain.checkSign(
            ioTransaction,
            serializedIncomingReceiptData.first,
            serializedIncomingReceiptData.second,
            kMessage->signature(),
            kMessage->publicKeyNumber())) {
            removeAllDataFromStorageConcerningTransaction(ioTransaction);
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Coordinator send invalid receipt signature. Rejected");
        }
        if (!keyChain.saveIncomingPaymentReceipt(
            ioTransaction,
            mTrustLinesManager->auditNumber(coordinatorID),
            mTransactionUUID,
            kMessage->publicKeyNumber(),
            coordinatorTotalIncomingReservationAmount,
            kMessage->signature())) {
            removeAllDataFromStorageConcerningTransaction(ioTransaction);
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Can't save coordinator receipt. Rejected.");
        }
        info() << "Coordinator's receipt is valid";
    } else {
        auto coordinatorID = mContractorsManager->contractorIDByAddress(coordinatorAddress);
        if (coordinatorID != ContractorsManager::kNotFoundContractorID) {
            auto coordinatorTotalIncomingReservationAmount = totalReservedIncomingAmountToNode(
                coordinatorID);
            if (coordinatorTotalIncomingReservationAmount != TrustLine::kZeroAmount()) {
                removeAllDataFromStorageConcerningTransaction(ioTransaction);
                sendErrorMessageOnFinalAmountsConfiguration();
                warning() << "Receipt amount: 0. Local reserved incoming amount: "
                          << coordinatorTotalIncomingReservationAmount;
                return reject("Coordinator send invalid receipt amount. Rejected");
            }
        }
    }

    mStep = Common_ObservingBlockNumberProcessing;
    mResourcesManager->requestObservingBlockNumber(
        mTransactionUUID);
    mBlockNumberObtainingInProcess = true;
    return resultWaitForResourceAndMessagesTypes(
        {BaseResource::ObservingBlockNumber},
        {Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(1));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runCheckObservingBlockNumber()
{
    info() << "runCheckObservingBlockNumber";
    if (contextIsValid(Message::Payments_TransactionPublicKeyHash, false)) {
        return runFinalReservationsNeighborConfirmation();
    }

    // todo : add Payments_TTLProlongationResponse checking

    if (!resourceIsValid(BaseResource::ObservingBlockNumber)) {
        removeAllDataFromStorageConcerningTransaction();
        sendErrorMessageOnFinalAmountsConfiguration();
        return reject("Can't check observing actual block number. Rejected.");
    }
    auto blockNumberResource = popNextResource<BlockNumberRecourse>();
    auto maximalClaimingBlockNumber = blockNumberResource->actualObservingBlockNumber() + kCountBlocksForClaiming;
    debug() << "maximal claiming block number on own side: " << maximalClaimingBlockNumber;
    if (!checkMaxClaimingBlockNumber(maximalClaimingBlockNumber)) {
        removeAllDataFromStorageConcerningTransaction();
        sendErrorMessageOnFinalAmountsConfiguration();
        return reject("Max claiming block number sending by coordinator is invalid. Rejected.");
    }
    mBlockNumberObtainingInProcess = false;

    auto ioTransaction = mStorageHandler->beginTransaction();
    mPublicKey = mKeysStore->generateAndSaveKeyPairForPaymentTransaction(
        ioTransaction,
        currentTransactionUUID());
    mParticipantsPublicKeysHashes.insert(
        make_pair(
            mContractorsManager->selfContractor()->mainAddress()->fullAddress(),
            make_pair(
                mPaymentNodesIds[mContractorsManager->selfContractor()->mainAddress()->fullAddress()],
                mPublicKey->hash())));

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnVoteStage(
        (uint32_t)mPaymentParticipants.size() - 2);
#endif

    // send messages to all participants except coordinator:
    // to nodes with outgoing reservations - outgoing receipts and public key hash;
    // to rest nodes - only public key hash
    auto ownPaymentID = mPaymentNodesIds[mContractorsManager->selfContractor()->mainAddress()->fullAddress()];
    for (const auto &nodePaymentIdAndContractor : mPaymentParticipants) {
        if (nodePaymentIdAndContractor.second == mCoordinator) {
            continue;
        }
        if (nodePaymentIdAndContractor.second == mContractorsManager->selfContractor()) {
            continue;
        }

        auto participantID = mContractorsManager->contractorIDByAddress(nodePaymentIdAndContractor.second->mainAddress());
        if (mReservations.find(participantID) == mReservations.end()) {
            info() << "Send public key hash to " << nodePaymentIdAndContractor.second->mainAddress()->fullAddress();
            sendMessage<TransactionPublicKeyHashMessage>(
                nodePaymentIdAndContractor.second->mainAddress(),
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                ownPaymentID,
                mPublicKey->hash());
            continue;
        }
        auto nodeReservations = mReservations[participantID];
        if (nodeReservations.begin()->second->direction() == AmountReservation::Outgoing) {
            auto keyChain = mKeysStore->keychain(
                mTrustLinesManager->trustLineID(participantID));
            auto outgoingReservedAmount = TrustLine::kZeroAmount();
            for (const auto &pathIDAndReservation : nodeReservations) {
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
                removeAllDataFromStorageConcerningTransaction(ioTransaction);
                sendErrorMessageOnFinalAmountsConfiguration();
                return reject("Can't save outgoing receipt. Rejected.");
            }
            info() << "Send public key hash to " << nodePaymentIdAndContractor.second->mainAddress()->fullAddress()
                   << " with receipt " << outgoingReservedAmount;
            sendMessage<TransactionPublicKeyHashMessage>(
                nodePaymentIdAndContractor.second->mainAddress(),
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                ownPaymentID,
                mPublicKey->hash(),
                signatureAndKeyNumber.second,
                signatureAndKeyNumber.first);
        } else {
            info() << "Send public key hash to " << nodePaymentIdAndContractor.second->mainAddress()->fullAddress();
            sendMessage<TransactionPublicKeyHashMessage>(
                nodePaymentIdAndContractor.second->mainAddress(),
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                ownPaymentID,
                mPublicKey->hash());
        }
    }

    // coordinator don't send public key hash
    if (mPaymentParticipants.size() == mParticipantsPublicKeysHashes.size() + 1) {
        // all neighbors sent theirs reservations
        if (!checkAllPublicKeyHashesProperly()) {
            removeAllDataFromStorageConcerningTransaction(ioTransaction);
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Public key hashes are not properly. Rejected");
        }

        sendMessage<FinalAmountsConfigurationResponseMessage>(
            mCoordinator->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Accepted,
            mPublicKey);

        info() << "Accepted final amounts configuration";

        mStep = Common_Voting;
        mTTLRequestWasSend = false;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsPublicKeys,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(5)); // todo : need discuss this parameter (5)
    }

    mStep = Coordinator_FinalAmountsConfigurationConfirmation;
    return resultWaitForMessageTypes(
        {Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runFinalReservationsNeighborConfirmation()
{
    debug() << "runFinalReservationsNeighborConfirmation";
    auto kMessage = popNextMessage<TransactionPublicKeyHashMessage>();
    auto senderAddress = kMessage->senderAddresses.at(0);
    debug() << "sender: " << senderAddress->fullAddress();

    mParticipantsPublicKeysHashes[senderAddress->fullAddress()] = make_pair(
        kMessage->paymentNodeID(),
        kMessage->transactionPublicKeyHash());

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (kMessage->isReceiptContains()) {
        info() << "Sender also send receipt";
        auto senderID = mContractorsManager->contractorIDByAddress(senderAddress);
        if (senderID == ContractorsManager::kNotFoundContractorID) {
            removeAllDataFromStorageConcerningTransaction(ioTransaction);
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Sender is not a neighbor. Rejected");
        }
        auto participantTotalIncomingReservationAmount = totalReservedIncomingAmountToNode(
            senderID);
        auto keyChain = mKeysStore->keychain(
            mTrustLinesManager->trustLineID(senderID));
        auto serializedIncomingReceiptData = getSerializedReceipt(
            senderID,
            mContractorsManager->idOnContractorSide(senderID),
            participantTotalIncomingReservationAmount,
            false);
        if (!keyChain.checkSign(
            ioTransaction,
            serializedIncomingReceiptData.first,
            serializedIncomingReceiptData.second,
            kMessage->signature(),
            kMessage->publicKeyNumber())) {
            removeAllDataFromStorageConcerningTransaction(ioTransaction);
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Sender send invalid receipt signature. Rejected");
        }
        if (!keyChain.saveIncomingPaymentReceipt(
            ioTransaction,
            mTrustLinesManager->auditNumber(senderID),
            mTransactionUUID,
            kMessage->publicKeyNumber(),
            participantTotalIncomingReservationAmount,
            kMessage->signature())) {
            removeAllDataFromStorageConcerningTransaction(ioTransaction);
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Can't save participant receipt. Rejected.");
        }
        info() << "Sender's receipt is valid";
    } else {
        auto senderID = mContractorsManager->contractorIDByAddress(senderAddress);
        if (senderID != ContractorsManager::kNotFoundContractorID) {
            auto participantTotalIncomingReservationAmount = totalReservedIncomingAmountToNode(
                senderID);
            if (participantTotalIncomingReservationAmount != TrustLine::kZeroAmount()) {
                removeAllDataFromStorageConcerningTransaction(ioTransaction);
                sendErrorMessageOnFinalAmountsConfiguration();
                warning() << "Receipt amount: 0. Local reserved incoming amount: "
                          << participantTotalIncomingReservationAmount;
                return reject("Sender send invalid receipt amount. Rejected");
            }
        }
    }

    // if coordinator don't sent final payment configuration yet
    if (mPaymentParticipants.empty()) {
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_TransactionPublicKeyHash,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(2));
    }

    if (mBlockNumberObtainingInProcess) {
        return resultWaitForResourceAndMessagesTypes(
            {BaseResource::ObservingBlockNumber},
            {Message::Payments_TransactionPublicKeyHash},
            maxNetworkDelay(1));
    }

    // coordinator already sent final amounts configuration
    // coordinator didn't send public key hash
    if (mPaymentParticipants.size() == mParticipantsPublicKeysHashes.size() + 1) {
        // all neighbors sent theirs reservations
        if (!checkAllPublicKeyHashesProperly()) {
            removeAllDataFromStorageConcerningTransaction(ioTransaction);
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Public key hashes are not properly. Rejected");
        }

        sendMessage<FinalAmountsConfigurationResponseMessage>(
            mCoordinator->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Accepted,
            mPublicKey);

        info() << "Accepted final amounts configuration";

        mStep = Common_Voting;
        mTTLRequestWasSend = false;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsPublicKeys,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(5)); // todo : need discuss this parameter (5)
    }

    // not all neighbors sent theirs reservations
    return resultWaitForMessageTypes(
        {Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runVotesCheckingStageWithCoordinatorClarification()
{
    info() << "runVotesCheckingStageWithCoordinatorClarification";
    if (contextIsValid(Message::Payments_ParticipantsPublicKeys, false)) {
        return runVotesCheckingStage();
    }

    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        debug() << "Receive TTL Message";
        const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
        if (kMessage->state() == TTLProlongationResponseMessage::Finish) {
            removeAllDataFromStorageConcerningTransaction();
            return reject("Coordinator send response with transaction finish state. Rolling Back");
        }
        // transactions is still alive and we continue waiting for messages
        debug() << "Transactions is still alive. Continue waiting for messages";
        if (utc_now() - mTimeStarted > kMaxTransactionDuration()) {
            return reject("Transaction duration time has expired. Rolling back");
        }
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsPublicKeys,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    if (mTTLRequestWasSend) {
        removeAllDataFromStorageConcerningTransaction();
        return reject("No participants votes message received. Transaction will be closed. Rolling Back");
    }

    debug() << "Send TTLTransaction message to coordinator " << mCoordinator->mainAddress()->fullAddress();
    sendMessage<TTLProlongationRequestMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID());
    mTTLRequestWasSend = true;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsPublicKeys,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

void IntermediateNodePaymentTransaction::shortageReservationsOnPath(
    const PathID pathID,
    const TrustLineAmount &amount)
{
    // Shortening all reservations that belongs to given path if they are different from new amount
    for (const auto &nodeAndReservations : mReservations) {
        for (const auto &pathIDAndReservation : nodeAndReservations.second) {
            if (pathIDAndReservation.first == pathID) {
                if (pathIDAndReservation.second->amount() != amount) {
                    shortageReservation(
                        nodeAndReservations.first,
                        pathIDAndReservation.second,
                        amount,
                        pathIDAndReservation.first);
                }
            }
        }
    }
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::approve()
{
    mCommittedAmount = totalReservedAmount(
            AmountReservation::Outgoing);
    BasePaymentTransaction::approve();
    BasePaymentTransaction::runThreeNodesCyclesTransactions();
    BasePaymentTransaction::runFourNodesCyclesTransactions();
    return resultDone();
}

BaseAddress::Shared IntermediateNodePaymentTransaction::coordinatorAddress() const
{
    return mCoordinator->mainAddress();
}

void IntermediateNodePaymentTransaction::savePaymentOperationIntoHistory(
    IOTransaction::Shared ioTransaction)
{
    debug() << "savePaymentOperationIntoHistory";
    ioTransaction->historyStorage()->savePaymentAdditionalRecord(
        make_shared<PaymentAdditionalRecord>(
            currentTransactionUUID(),
            PaymentAdditionalRecord::IntermediatePaymentType,
            mCommittedAmount),
        mEquivalent);
    debug() << "Operation saved";
}

bool IntermediateNodePaymentTransaction::checkReservationsDirections() const
{
    debug() << "checkReservationsDirections";
    for (const auto &nodeIDAndReservations : mReservations) {
        for (const auto &pathIDAndReservation : nodeIDAndReservations.second) {
            const auto checkedPath = pathIDAndReservation.first;
            const auto checkedAmount = pathIDAndReservation.second->amount();
            int countIncomingReservations = 0;
            int countOutgoingReservations = 0;

            for (const auto &nodeIDAndReservationsInternal : mReservations) {
                for (const auto &pathIDAndReservationInternal : nodeIDAndReservationsInternal.second) {
                    if (pathIDAndReservationInternal.first == checkedPath) {
                        if (pathIDAndReservationInternal.second->amount() != checkedAmount) {
                            warning() << "Amounts are different on path " << checkedPath;
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
                warning() << "Count incoming and outgoing reservations are invalid on path " << checkedPath;
                return false;
            }
        }
    }
    debug() << "All reservations directions are correct";
    return true;
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::sendErrorMessageOnPreviousNodeRequest(
    BaseAddress::Shared previousNode,
    PathID pathID,
    ResponseMessage::OperationState errorState)
{
    sendMessage<IntermediateNodeReservationResponseMessage>(
        previousNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        pathID,
        errorState);
    if (mReservations.empty()) {
        debug() << "There are no reservations. Transaction closed.";
        return resultDone();
    }
    mStep = Stages::IntermediateNode_ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TransactionPublicKeyHash,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::sendErrorMessageOnCoordinatorRequest(
    ResponseMessage::OperationState errorState)
{
    sendMessage<CoordinatorReservationResponseMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        mLastProcessedPath,
        errorState);
    rollBack(mLastProcessedPath);
    // if no reservations close transaction
    if (mReservations.empty()) {
        debug() << "There are no reservations. Transaction closed.";
        return resultDone();
    }
    mStep = Stages::IntermediateNode_ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TransactionPublicKeyHash,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::sendErrorMessageOnNextNodeResponse(
    ResponseMessage::OperationState errorState)
{
    sendMessage<CoordinatorReservationResponseMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        mLastProcessedPath,
        errorState);
    rollBack(mLastProcessedPath);
    // if no reservations close transaction
    if (mReservations.empty()) {
        debug() << "There are no reservations. Transaction closed.";
        return resultDone();
    }
    mStep = Stages::IntermediateNode_ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TransactionPublicKeyHash,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

void IntermediateNodePaymentTransaction::sendErrorMessageOnFinalAmountsConfiguration()
{
    sendMessage<FinalAmountsConfigurationResponseMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        FinalAmountsConfigurationResponseMessage::Rejected);
}

const string IntermediateNodePaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[IntermediateNodePaymentTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}