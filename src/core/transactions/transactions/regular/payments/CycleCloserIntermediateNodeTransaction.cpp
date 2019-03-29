#include "CycleCloserIntermediateNodeTransaction.h"

CycleCloserIntermediateNodeTransaction::CycleCloserIntermediateNodeTransaction(
    IntermediateNodeCycleReservationRequestMessage::ConstShared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager* trustLines,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    ResourcesManager *resourcesManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BasePaymentTransaction(
        BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction,
        message->transactionUUID(),
        message->equivalent(),
        false,
        contractorsManager,
        trustLines,
        storageHandler,
        topologyCacheManager,
        maxFlowCacheManager,
        resourcesManager,
        keystore,
        log,
        subsystemsController),
    mMessage(message),
    mCyclesManager(cyclesManager)
{
    mStep = Stages::IntermediateNode_PreviousNeighborRequestProcessing;
}

CycleCloserIntermediateNodeTransaction::CycleCloserIntermediateNodeTransaction(
    BytesShared buffer,
    ContractorsManager *contractorsManager,
    TrustLinesManager* trustLines,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    ResourcesManager *resourcesManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BasePaymentTransaction(
        buffer,
        false,
        contractorsManager,
        trustLines,
        storageHandler,
        topologyCacheManager,
        maxFlowCacheManager,
        resourcesManager,
        keystore,
        log,
        subsystemsController),
    mCyclesManager(cyclesManager)
{}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::run()
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

            case Stages::Common_ObservingBlockNumberProcessing:
                return runCheckObservingBlockNumber();

            case Stages::Common_Voting:
                return runVotesCheckingStageWithPossibleTTL();

            case Stages::Common_VotesChecking:
                return runVotesConsistencyCheckingStage();

            case Stages::Common_Recovery:
                return runVotesRecoveryParentStage();

            case Stages::Cycles_WaitForIncomingAmountReleasing:
                return runPreviousNeighborRequestProcessingStageAgain();

            case Stages::Cycles_WaitForOutgoingAmountReleasing:
                return runCoordinatorRequestProcessingStageAgain();

            case Stages::Common_RollbackByOtherTransaction:
                return runRollbackByOtherTransactionStage();

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

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runPreviousNeighborRequestProcessingStage()
{
    debug() << "runPreviousNeighborRequestProcessingStage";
    mPreviousNode = mMessage->senderAddresses.at(0);
    mCoordinator = mMessage->coordinatorAddress();
    debug() << "Init. intermediate payment operation from node (" << mPreviousNode->fullAddress() << ")";
    debug() << "Requested amount reservation: " << mMessage->amount();
    auto previousNodeID = mContractorsManager->contractorIDByAddress(mPreviousNode);
    if (previousNodeID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Previous node is not a neighbor";
        return sendErrorMessageOnPreviousNodeRequest(
            ResponseCycleMessage::Rejected);
    }
    info() << "Previous node ID: " << previousNodeID;

    mCycleLength = mMessage->cycleLength();

    if (!mTrustLinesManager->trustLineIsPresent(previousNodeID)) {
        warning() << "Path is not valid: previous node is not neighbor of current one. Rejected.";
        return sendErrorMessageOnPreviousNodeRequest(
            ResponseCycleMessage::Rejected);
    }

    if (!mTrustLinesManager->trustLineContractorKeysPresent(previousNodeID)) {
        warning() << "There are no contractor keys on TL";
        return sendErrorMessageOnPreviousNodeRequest(
            ResponseCycleMessage::RejectedDueContractorKeysAbsence);
    }

    // Note: (copy of shared pointer is required)
    const auto kIncomingAmounts = mTrustLinesManager->availableIncomingCycleAmounts(previousNodeID);
    const auto kIncomingAmountWithReservations = kIncomingAmounts.first;
    const auto kIncomingAmountWithoutReservations = kIncomingAmounts.second;

    debug() << "IncomingAmountWithReservations: " << *kIncomingAmountWithReservations
            << " IncomingAmountWithoutReservations: " << *kIncomingAmountWithoutReservations;
    if (*kIncomingAmountWithReservations == TrustLine::kZeroAmount()) {
        if (*kIncomingAmountWithoutReservations == TrustLine::kZeroAmount()) {
            warning() << "can't reserve requested amount, event with reservations, transaction closed";
            return sendErrorMessageOnPreviousNodeRequest(
                ResponseCycleMessage::Rejected);
        } else {
            mReservationAmount = TrustLineAmount(0);
        }
    } else {
        mReservationAmount = min(
            mMessage->amount(),
            *kIncomingAmountWithReservations);
    }

    if (0 == mReservationAmount) {
        // try to use reservations from other transactions
        auto reservations = mTrustLinesManager->reservationsFromContractor(previousNodeID);
        for (auto &reservation : reservations) {
            debug() << "try use " << reservation->amount() << " from "
                    << reservation->transactionUUID() << " transaction";
            if (mCyclesManager->resolveReservationConflict(
                currentTransactionUUID(), reservation->transactionUUID())) {
                debug() << "win reservation";
                mConflictedTransaction = reservation->transactionUUID();
                mStep = Cycles_WaitForIncomingAmountReleasing;
                mReservationAmount = min(
                    mMessage->amount(),
                    reservation->amount());
                return resultAwakeAfterMilliseconds(
                    kWaitingForReleasingAmountMSec);
            }
            debug() << "don't win reservation";
        }
    }

    if (0 == mReservationAmount || ! reserveIncomingAmount(previousNodeID, mReservationAmount, 0)) {
        warning() << "can't reserve requested amount, transaction closed";
        return sendErrorMessageOnPreviousNodeRequest(
            ResponseCycleMessage::RejectedBecauseReservations);
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
        mPreviousNode,
        mReservationAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    mLastReservedAmount = mReservationAmount;
    debug() << "send accepted message with reserve (" << mReservationAmount << ")";
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        ResponseCycleMessage::Accepted,
        mReservationAmount);

    mStep = Stages::IntermediateNode_CoordinatorRequestProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationRequest,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(3));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runPreviousNeighborRequestProcessingStageAgain()
{
    debug() << "runPreviousNeighborRequestProcessingStageAgain";
    if (mCyclesManager->isTransactionStillAlive(
        mConflictedTransaction)) {
        debug() << "wait again";
        return resultAwakeAfterMilliseconds(
            kWaitingForReleasingAmountMSec);
    }
    debug() << "try reserve " << mReservationAmount;
    auto previousNodeID = mContractorsManager->contractorIDByAddress(mPreviousNode);
    if (! reserveIncomingAmount(previousNodeID, mReservationAmount, 0)) {
        warning() << "can't reserve requested amount, transaction closed";
        return sendErrorMessageOnPreviousNodeRequest(
            ResponseCycleMessage::RejectedBecauseReservations);
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
        mPreviousNode,
        mReservationAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    mLastReservedAmount = mReservationAmount;
    debug() << "send accepted message with reserve (" << mReservationAmount << ")";
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        ResponseCycleMessage::Accepted,
        mReservationAmount);

    mStep = Stages::IntermediateNode_CoordinatorRequestProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationRequest,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(3));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runCoordinatorRequestProcessingStage()
{
    debug() << "runCoordinatorRequestProcessingStage";
    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        debug() << "Receive TTL Finish message. Rolled back.";
        clearContext();
        rollBack();
        return resultDone();
    }

    if (!contextIsValid(Message::Payments_CoordinatorCycleReservationRequest)) {
        return reject("No coordinator request received. Rolled back.");
    }

    debug() << "Coordinator further reservation request received.";

    // TODO: add check for previous nodes amount reservation

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
        nullptr,
        0);
    mSubsystemsController->testThrowExceptionOnCoordinatorRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnCoordinatorRequestProcessingStage();
#endif

    const auto kMessage = popNextMessage<CoordinatorCycleReservationRequestMessage>();
    if (kMessage->senderAddresses.at(0) != mCoordinator) {
        warning() << "Sender " << kMessage->senderAddresses.at(0)->fullAddress() << " is not coordinator";
        return resultContinuePreviousState();
    }
    mNextNode = kMessage->nextNodeInPathAddress();

    debug() << "Requested amount reservation: " << kMessage->amount();
    debug() << "Next node is " << mNextNode->fullAddress();
    auto nextNodeID = mContractorsManager->contractorIDByAddress(mNextNode);
    if (nextNodeID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Next node is not a neighbor";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseCycleMessage::Rejected);
    }

    if (!mTrustLinesManager->trustLineIsPresent(nextNodeID)) {
        warning() << "Path is not valid: next node is not neighbor of current one. Rejected.";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseCycleMessage::Rejected);
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLinesManager->trustLineOwnKeysPresent(nextNodeID)) {
        warning() << "There are no own keys on TL";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseCycleMessage::RejectedDueOwnKeysAbsence);
    }

    // Note: copy of shared pointer is required
    const auto kOutgoingAmounts = mTrustLinesManager->availableOutgoingCycleAmounts(nextNodeID);
    const auto kOutgoingAmountWithReservations = kOutgoingAmounts.first;
    const auto kOutgoingAmountWithoutReservations = kOutgoingAmounts.second;
    debug() << "OutgoingAmountWithReservations: " << *kOutgoingAmountWithReservations
            << " OutgoingAmountWithoutReservations: " << *kOutgoingAmountWithoutReservations;

    if (*kOutgoingAmountWithReservations == TrustLine::kZeroAmount()) {
        if (*kOutgoingAmountWithoutReservations == TrustLine::kZeroAmount()) {
            warning() << "No amount reservation is possible, even with using other reservations. Rolled back.";
            return sendErrorMessageOnCoordinatorRequest(
                ResponseCycleMessage::Rejected);
        } else {
            mReservationAmount = TrustLineAmount(0);
        }
    } else {
        mReservationAmount = min(
            kMessage->amount(),
            *kOutgoingAmountWithReservations);
    }

    if (0 == mReservationAmount) {
        // try to use reservation from other transaction
        auto reservations = mTrustLinesManager->reservationsToContractor(nextNodeID);
        for (auto &reservation : reservations) {
            debug() << "try use " << reservation->amount() << " from "
                    << reservation->transactionUUID() << " transaction";
            if (mCyclesManager->resolveReservationConflict(
                currentTransactionUUID(), reservation->transactionUUID())) {
                debug() << "win reservation";
                mConflictedTransaction = reservation->transactionUUID();
                mStep = Cycles_WaitForOutgoingAmountReleasing;
                mReservationAmount = min(
                    kMessage->amount(),
                    reservation->amount());
                return resultAwakeAfterMilliseconds(
                    kWaitingForReleasingAmountMSec);
            }
            debug() << "don't win reservation";
        }
    }
    if (0 == mReservationAmount || ! reserveOutgoingAmount(nextNodeID, mReservationAmount, 0)) {
        warning() << "No amount reservation is possible. Rolled back.";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseCycleMessage::RejectedBecauseReservations);
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendRequestToIntNodeOnReservationStage(
        mNextNode,
        mReservationAmount);
#endif

    mLastReservedAmount = mReservationAmount;
    debug() << "Send request reservation on " << mReservationAmount << " to next node";
    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        mNextNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        mReservationAmount,
        mCoordinator,
        mCycleLength);

    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse,
         Message::Payments_TTLProlongationResponse,
         Message::General_NoEquivalent},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runCoordinatorRequestProcessingStageAgain()
{
    debug() << "runCoordinatorRequestProcessingStageAgain";
    if (mCyclesManager->isTransactionStillAlive(
        mConflictedTransaction)) {
        debug() << "wait again";
        return resultAwakeAfterMilliseconds(
            kWaitingForReleasingAmountMSec);
    }
    debug() << "try reserve " << mReservationAmount;
    auto nextNodeID = mContractorsManager->contractorIDByAddress(mNextNode);
    if (! reserveOutgoingAmount(nextNodeID, mReservationAmount, 0)) {
        warning() << "No amount reservation is possible. Rolled back.";
        return sendErrorMessageOnCoordinatorRequest(
            ResponseCycleMessage::RejectedBecauseReservations);
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendRequestToIntNodeOnReservationStage(
        mNextNode,
        mReservationAmount);
#endif

    mLastReservedAmount = mReservationAmount;
    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        mNextNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        mReservationAmount,
        mCoordinator,
        mCycleLength);

    clearContext();
    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse,
         Message::Payments_TTLProlongationResponse,
         Message::General_NoEquivalent},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runNextNeighborResponseProcessingStage()
{
    debug() << "runNextNeighborResponseProcessingStage";
    if (contextIsValid(Message::General_NoEquivalent, false)) {
        warning() << "Neighbour hasn't TLs on requested equivalent. Canceling.";
        return sendErrorMessageOnNextNodeResponse(
            ResponseCycleMessage::Rejected);
    }
    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        debug() << "Receive TTL Finish message. Rolled back.";
        clearContext();
        rollBack();
        return resultDone();
    }
    if (! contextIsValid(Message::Payments_IntermediateNodeCycleReservationResponse)) {
        warning() << "No valid amount reservation response received. Rolled back.";
        return sendErrorMessageOnNextNodeResponse(
            ResponseCycleMessage::NextNodeInaccessible);
    }

    const auto kMessage = popNextMessage<IntermediateNodeCycleReservationResponseMessage>();
    if (kMessage->senderAddresses.at(0) != mNextNode) {
        warning() << "Sender " << kMessage->senderAddresses.at(0)->fullAddress() << " is not a next neighbor";
        return resultContinuePreviousState();
    }

    if (kMessage->state() == IntermediateNodeCycleReservationResponseMessage::RejectedDueContractorKeysAbsence){
        warning() << "Next node doesn't approved reservation request due to contractor keys absence";
        // todo maybe set mOwnKeysPresent into false and initiate KeysSharing TA
        return sendErrorMessageOnNextNodeResponse(
            ResponseCycleMessage::RejectedDueContractorKeysAbsence);
    }

    if (kMessage->state() == IntermediateNodeCycleReservationResponseMessage::Rejected ||
            kMessage->state() == IntermediateNodeCycleReservationResponseMessage::RejectedBecauseReservations){
        warning() << "Amount reservation rejected by the neighbor node.";
        return sendErrorMessageOnNextNodeResponse(
            kMessage->state());
    }

    if (kMessage->state() != IntermediateNodeCycleReservationResponseMessage::Accepted) {
        warning() << "Unexpected message state. Protocol error.";
        // state RejectedBecauseReservations is needed for prevent
        // add offline nodes and closed trustlines on coordinator
        return sendErrorMessageOnNextNodeResponse(
            ResponseCycleMessage::RejectedBecauseReservations);
    }

    debug() << "Next node accepted amount reservation.";
    mLastReservedAmount = kMessage->amountReserved();
    // shortage local reservation on current path
    for (const auto &nodeReservation : mReservations) {
        for (const auto &pathIDAndReservation : nodeReservation.second) {
            shortageReservation(
                nodeReservation.first,
                pathIDAndReservation.second,
                mLastReservedAmount,
                pathIDAndReservation.first);
        }
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
        mCoordinator,
        mLastReservedAmount);
    mSubsystemsController->testThrowExceptionOnNextNeighborResponseProcessingStage();
    mSubsystemsController->testTerminateProcessOnNextNeighborResponseProcessingStage();
#endif

    debug() << "send accepted message with reserve (" << mLastReservedAmount << ") to coordinator";
    sendMessage<CoordinatorCycleReservationResponseMessage>(
        mCoordinator,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        ResponseCycleMessage::Accepted,
        mLastReservedAmount);

    mStep = Stages::Common_FinalPathConfigurationChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalPathCycleConfiguration,
         Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runFinalPathConfigurationProcessingStage()
{
    debug() << "runFinalPathConfigurationProcessingStage";
    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        debug() << "Receive TTL Finish message. Rolled back.";
        clearContext();
        rollBack();
        removeAllDataFromStorageConcerningTransaction();
        return resultDone();
    }

    if (contextIsValid(Message::Payments_FinalPathCycleConfiguration, false)) {
        return runFinalPathConfigurationCoordinatorConfirmation();
    }

    // todo make custom Payments_TransactionPublicKeyHash message for cycles with only one receipt
    if (contextIsValid(Message::Payments_TransactionPublicKeyHash, false)) {
        return runFinalReservationsNeighborConfirmation();
    }

    removeAllDataFromStorageConcerningTransaction();
    return reject("No final paths configuration was received from the coordinator. Rejected.");
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runFinalPathConfigurationCoordinatorConfirmation()
{
    debug() << "runFinalPathConfigurationCoordinatorConfirmation";
    const auto kMessage = popNextMessage<FinalPathCycleConfigurationMessage>();
    // todo : check if sender is coordinator

    debug() << "Final payment path configuration received";
    mMaximalClaimingBlockNumber = kMessage->maximalClaimingBlockNumber();
    debug() << "maximal claiming block number: " << mMaximalClaimingBlockNumber;

    // path was cancelled, drop all reservations belong it
    if (kMessage->amount() == 0) {
        debug() << "Final payment equals 0, transaction cancelled.";
        rollBack();
        removeAllDataFromStorageConcerningTransaction();
        return resultDone();
    }

    mLastReservedAmount = kMessage->amount();
    // Shortening all reservations that belongs to this node and path.
    for (const auto &nodeAndReservations : mReservations) {
        for (const auto &pathIDAndReservation : nodeAndReservations.second) {
            if (pathIDAndReservation.second->amount() != kMessage->amount()) {
                shortageReservation(
                    nodeAndReservations.first,
                    pathIDAndReservation.second,
                    kMessage->amount(),
                    pathIDAndReservation.first);
            }
        }
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnFinalAmountClarificationStage();
#endif

    debug() << "All reservations were updated";
    if (!checkReservationsDirections()) {
        removeAllDataFromStorageConcerningTransaction();
        sendErrorMessageOnFinalAmountsConfiguration();
        return reject("Reservations on node are invalid");
    }
    info() << "All reservations are correct";

    mPaymentParticipants = kMessage->paymentParticipants();
    // todo check if current node is present in mPaymentNodesIds
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
        auto coordinatorID = mContractorsManager->contractorIDByAddress(mCoordinator);
        if (coordinatorID == ContractorsManager::kNotFoundContractorID) {
            removeAllDataFromStorageConcerningTransaction(ioTransaction);
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Coordinator is not a neighbor. Rejected");
        }
        auto keyChain = mKeysStore->keychain(
            mTrustLinesManager->trustLineID(coordinatorID));
        auto coordinatorTotalIncomingReservationAmount = totalReservedIncomingAmountToNode(
            coordinatorID);
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
        auto coordinatorID = mContractorsManager->contractorIDByAddress(mCoordinator);
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

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runCheckObservingBlockNumber()
{
    info() << "runCheckObservingBlockNumber";
    if (contextIsValid(Message::Payments_TransactionPublicKeyHash, false)) {
        return runFinalReservationsNeighborConfirmation();
    }

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
        return reject("Max claiming block number sending by coordinator is invalid . Rejected.");
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
    for (const auto &paymentNodeIdAndContractor : mPaymentParticipants) {
        if (paymentNodeIdAndContractor.second == mContractorsManager->selfContractor()) {
            continue;
        }
        auto participantID = mContractorsManager->contractorIDByAddress(paymentNodeIdAndContractor.second->mainAddress());
        if (mReservations.find(participantID) == mReservations.end()) {
            if (paymentNodeIdAndContractor.first == kCoordinatorPaymentNodeID) {
                continue;
            }
            sendMessage<TransactionPublicKeyHashMessage>(
                paymentNodeIdAndContractor.second->mainAddress(),
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                ownPaymentID,
                mPublicKey->hash());
            continue;
        }
        auto nodeReservations = mReservations[participantID];
        if (nodeReservations.begin()->second->direction() == AmountReservation::Outgoing) {
            // in cycles nodes should have only one outgoing reservation
            auto outgoingReservedAmount = nodeReservations.begin()->second->amount();
            auto keyChain = mKeysStore->keychain(
                mTrustLinesManager->trustLineID(participantID));
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
            sendMessage<TransactionPublicKeyHashMessage>(
                paymentNodeIdAndContractor.second->mainAddress(),
                mEquivalent,
                mContractorsManager->ownAddresses(),
                currentTransactionUUID(),
                ownPaymentID,
                mPublicKey->hash(),
                signatureAndKeyNumber.second,
                signatureAndKeyNumber.first);
        } else {
            if (paymentNodeIdAndContractor.first == kCoordinatorPaymentNodeID) {
                continue;
            }
            sendMessage<TransactionPublicKeyHashMessage>(
                paymentNodeIdAndContractor.second->mainAddress(),
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
            mCoordinator,
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Accepted,
            mPublicKey);

        info() << "Accepted final amounts configuration";

        mStep = Common_Voting;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsPublicKeys,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(5)); // todo : need discuss this parameter (5)
    }

    mStep = Common_FinalPathConfigurationChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runFinalReservationsNeighborConfirmation()
{
    debug() << "runFinalReservationsNeighborConfirmation";
    auto kMessage = popNextMessage<TransactionPublicKeyHashMessage>();
    auto senderAddress = kMessage->senderAddresses.at(0);
    debug() << "sender: " << senderAddress->fullAddress();

    mParticipantsPublicKeysHashes[senderAddress->fullAddress()] = make_pair(
        kMessage->paymentNodeID(),
        kMessage->transactionPublicKeyHash());

    if (kMessage->isReceiptContains()) {
        info() << "Sender also send receipt";
        auto ioTransaction = mStorageHandler->beginTransaction();
        // todo : add try catch IOError
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
                removeAllDataFromStorageConcerningTransaction();
                sendErrorMessageOnFinalAmountsConfiguration();
                warning() << "Receipt amount: 0. Local reserved incoming amount: "
                          << participantTotalIncomingReservationAmount;
                return reject("Sender send invalid receipt amount. Rejected");
            }
        }
    }

    // if coordinator didn't sent final payment configuration yet
    if (mPaymentParticipants.empty()) {
        return resultWaitForMessageTypes(
            {Message::Payments_FinalPathCycleConfiguration,
             Message::Payments_TransactionPublicKeyHash,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(1));
    }

    if (mBlockNumberObtainingInProcess) {
        return resultWaitForResourceAndMessagesTypes(
            {BaseResource::ObservingBlockNumber},
            {Message::Payments_TransactionPublicKeyHash},
            maxNetworkDelay(1));
    }

    // coordinator already sent final amounts configuration
    // coordinator don't send public key hash
    if (mPaymentParticipants.size() == mParticipantsPublicKeysHashes.size() + 1) {
        // all neighbors sent theirs reservations
        if (!checkAllPublicKeyHashesProperly()) {
            removeAllDataFromStorageConcerningTransaction();
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Public key hashes are not properly. Rejected");
        }

        sendMessage<FinalAmountsConfigurationResponseMessage>(
            mCoordinator,
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Accepted,
            mPublicKey);

        info() << "Accepted final amounts configuration";

        mStep = Common_Voting;
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

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runVotesCheckingStageWithPossibleTTL()
{
    debug() << "runVotesCheckingStageWithPossibleTTL";
    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        if (mTransactionIsVoted) {
            return recover("Unexpected message receive after voting. Protocol error.");
        }
        debug() << "Receive TTL Finish message. Rolled back.";
        clearContext();
        rollBack();
        removeAllDataFromStorageConcerningTransaction();
        return resultDone();
    }

    if (!contextIsValid(Message::Payments_ParticipantsPublicKeys, false) and
            !contextIsValid(Message::Payments_ParticipantsVotes)) {
        if (mTransactionIsVoted) {
            return recover("No participants votes message with all votes received.");
        } else {
            removeAllDataFromStorageConcerningTransaction();
            return reject("No participants votes message received. Transaction was closed. Rolling Back");
        }
    }

    return runVotesCheckingStage();
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runRollbackByOtherTransactionStage()
{
    debug() << "runRollbackByOtherTransactionStage";
    rollBack();
    return resultDone();
}

BaseAddress::Shared CycleCloserIntermediateNodeTransaction::coordinatorAddress() const
{
    return mCoordinator;
}

const SerializedPathLengthSize CycleCloserIntermediateNodeTransaction::cycleLength() const
{
    return mCycleLength;
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::approve()
{
    mCommittedAmount = totalReservedAmount(
            AmountReservation::Outgoing);
    return BasePaymentTransaction::approve();
}

void CycleCloserIntermediateNodeTransaction::savePaymentOperationIntoHistory(
    IOTransaction::Shared ioTransaction)
{
    debug() << "savePaymentOperationIntoHistory";
    ioTransaction->historyStorage()->savePaymentAdditionalRecord(
        make_shared<PaymentAdditionalRecord>(
            currentTransactionUUID(),
            PaymentAdditionalRecord::CycleCloserIntermediateType,
            mCommittedAmount),
        mEquivalent);
    debug() << "Operation saved";
}

bool CycleCloserIntermediateNodeTransaction::checkReservationsDirections() const
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

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::sendErrorMessageOnPreviousNodeRequest(
    ResponseCycleMessage::OperationState errorState)
{
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        errorState);
    return resultDone();
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::sendErrorMessageOnCoordinatorRequest(
    ResponseCycleMessage::OperationState errorState)
{
    sendMessage<CoordinatorCycleReservationResponseMessage>(
        mCoordinator,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        errorState);
    rollBack();
    return resultDone();
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::sendErrorMessageOnNextNodeResponse(
    ResponseCycleMessage::OperationState errorState)
{
    sendMessage<CoordinatorCycleReservationResponseMessage>(
        mCoordinator,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        errorState);
    rollBack();
    return resultDone();
}

void CycleCloserIntermediateNodeTransaction::sendErrorMessageOnFinalAmountsConfiguration()
{
    sendMessage<FinalAmountsConfigurationResponseMessage>(
        mCoordinator,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        FinalAmountsConfigurationResponseMessage::Rejected);
}

const string CycleCloserIntermediateNodeTransaction::logHeader() const
{
    stringstream s;
    s << "[CycleCloserIntermediateNodeTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
