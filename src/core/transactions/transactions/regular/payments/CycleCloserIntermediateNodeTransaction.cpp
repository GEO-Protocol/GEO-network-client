#include "CycleCloserIntermediateNodeTransaction.h"

CycleCloserIntermediateNodeTransaction::CycleCloserIntermediateNodeTransaction(
    const NodeUUID& currentNodeUUID,
    IntermediateNodeCycleReservationRequestMessage::ConstShared message,
    TrustLinesManager* trustLines,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BasePaymentTransaction(
        BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        message->equivalent(),
        trustLines,
        storageHandler,
        topologyCacheManager,
        maxFlowCacheManager,
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
    const NodeUUID &nodeUUID,
    TrustLinesManager* trustLines,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

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

            case Stages::Common_VotesChecking:
                return runVotesCheckingStageWithPossibleTTL();

            case Stages::Common_Recovery:
                return runVotesRecoveryParentStage();

            case Stages::Cycles_WaitForIncomingAmountReleasing:
                return runPreviousNeighborRequestProcessingStageAgain();

            case Stages::Cycles_WaitForOutgoingAmountReleasing:
                return runCoordinatorRequestProcessingStageAgain();

            case Stages::Common_RollbackByOtherTransaction:
                return runRollbackByOtherTransactionStage();

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
    mPreviousNode = mMessage->senderUUID;
    mCoordinator = mMessage->coordinatorUUID();
    debug() << "Init. intermediate payment operation from node (" << mPreviousNode << ")";
    debug() << "Requested amount reservation: " << mMessage->amount();

    mCycleLength = mMessage->cycleLength();

    if (!mTrustLines->trustLineIsPresent(mPreviousNode)) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            mPreviousNode,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);
        warning() << "Path is not valid: previous node is not neighbor of current one. Rejected.";
        return resultDone();
    }

    // Note: (copy of shared pointer is required)
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
            warning() << "can't reserve requested amount, event with reservations, transaction closed";
            return resultDone();
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
        auto reservations = mTrustLines->reservationsFromContractor(mPreviousNode);
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

    if (0 == mReservationAmount || ! reserveIncomingAmount(mPreviousNode, mReservationAmount, 0)) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            mPreviousNode,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::RejectedBecauseReservations);
        warning() << "can't reserve requested amount, transaction closed";
        return resultDone();
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
        mPreviousNode,
        mReservationAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    mLastReservedAmount = mReservationAmount;
    debug() << "send accepted message with reserve (" << mReservationAmount << ") to node " << mPreviousNode;
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID(),
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
    if (! reserveIncomingAmount(mPreviousNode, mReservationAmount, 0)) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            mPreviousNode,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::RejectedBecauseReservations);
        warning() << "can't reserve requested amount, transaction closed";
        return resultDone();
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
        mPreviousNode,
        mReservationAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    mLastReservedAmount = mReservationAmount;
    debug() << "send accepted message with reserve (" << mReservationAmount << ") to node " << mPreviousNode;
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID(),
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

    if (!contextIsValid(Message::Payments_CoordinatorCycleReservationRequest))
        return reject("No coordinator request received. Rolled back.");

    debug() << "Coordinator further reservation request received.";

    // TODO: add check for previous nodes amount reservation

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
        NodeUUID::empty(),
        0);
    mSubsystemsController->testThrowExceptionOnCoordinatorRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnCoordinatorRequestProcessingStage();
#endif

    const auto kMessage = popNextMessage<CoordinatorCycleReservationRequestMessage>();
    mNextNode = kMessage->nextNodeInPath();

    debug() << "Requested amount reservation: " << kMessage->amount();
    debug() << "Next node is " << kMessage->nextNodeInPath();

    if (!mTrustLines->trustLineIsPresent(mNextNode)) {
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);

        warning() << "Path is not valid: next node is not neighbor of current one. Rejected.";
        rollBack();
        return resultDone();
    }

    // Note: copy of shared pointer is required
    const auto kOutgoingAmounts = mTrustLines->availableOutgoingCycleAmounts(mNextNode);
    const auto kOutgoingAmountWithReservations = kOutgoingAmounts.first;
    const auto kOutgoingAmountWithoutReservations = kOutgoingAmounts.second;
    debug() << "OutgoingAmountWithReservations: " << *kOutgoingAmountWithReservations
            << " OutgoingAmountWithoutReservations: " << *kOutgoingAmountWithoutReservations;

    if (*kOutgoingAmountWithReservations == TrustLine::kZeroAmount()) {
        if (*kOutgoingAmountWithoutReservations == TrustLine::kZeroAmount()) {
            sendMessage<CoordinatorCycleReservationResponseMessage>(
                mCoordinator,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                ResponseCycleMessage::Rejected);

            warning() << "No amount reservation is possible, even with using other reservations. Rolled back.";
            rollBack();
            return resultDone();
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
        auto reservations = mTrustLines->reservationsToContractor(mNextNode);
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
    if (0 == mReservationAmount || ! reserveOutgoingAmount(mNextNode, mReservationAmount, 0)) {
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::RejectedBecauseReservations);

        warning() << "No amount reservation is possible. Rolled back.";
        rollBack();
        return resultDone();
    }

#ifdef TESTS
    mSubsystemsController->testForbidSendRequestToIntNodeOnReservationStage(
        mNextNode,
        mReservationAmount);
#endif

    mLastReservedAmount = mReservationAmount;
    debug() << "Send request reservation on " << mReservationAmount << " to " << mNextNode;
    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        mNextNode,
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID(),
        mReservationAmount,
        mCoordinator,
        mCycleLength);

    clearContext();
    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse,
         Message::NoEquivalent},
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
    if (! reserveOutgoingAmount(mNextNode, mReservationAmount, 0)) {
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::RejectedBecauseReservations);

        warning() << "No amount reservation is possible. Rolled back.";
        rollBack();
        return resultDone();
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
        currentNodeUUID(),
        currentTransactionUUID(),
        mReservationAmount,
        mCoordinator,
        mCycleLength);

    clearContext();
    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse,
         Message::NoEquivalent},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runNextNeighborResponseProcessingStage()
{
    debug() << "runNextNeighborResponseProcessingStage";
    if (contextIsValid(Message::NoEquivalent, false)) {
        warning() << "Neighbour hasn't TLs on requested equivalent. Canceling.";
        rollBack();
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);
        return resultDone();
    }
    if (! contextIsValid(Message::Payments_IntermediateNodeCycleReservationResponse)) {
        warning() << "No valid amount reservation response received. Rolled back.";
        rollBack();
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::NextNodeInaccessible);
        return resultDone();
    }

    const auto kMessage = popNextMessage<IntermediateNodeCycleReservationResponseMessage>();
    const auto kContractor = kMessage->senderUUID;

    if (kMessage->state() == IntermediateNodeCycleReservationResponseMessage::Rejected ||
            kMessage->state() == IntermediateNodeCycleReservationResponseMessage::RejectedBecauseReservations){
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            kMessage->state());
        warning() << "Amount reservation rejected by the neighbor node.";
        rollBack();
        return resultDone();
    }

    if (kMessage->state() != IntermediateNodeCycleReservationResponseMessage::Accepted) {
        warning() << "Unexpected message state. Protocol error.";
        rollBack();
        // state RejectedBecauseReservations is needed for prevent
        // add offline nodes and closed trustlines on coordinator
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::RejectedBecauseReservations);
        return resultDone();
    }

    debug() << "(" << kContractor << ") accepted amount reservation.";
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

    debug() << "send accepted message with reserve (" << mLastReservedAmount << ") to node " << mCoordinator;
    sendMessage<CoordinatorCycleReservationResponseMessage>(
        mCoordinator,
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID(),
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
        return resultDone();
    }

    if (contextIsValid(Message::Payments_FinalPathCycleConfiguration, false)) {
        return runFinalPathConfigurationCoordinatorConfirmation();
    }

    // todo make custom Payments_TransactionPublicKeyHash message for cycles with only one receipt
    if (contextIsValid(Message::Payments_TransactionPublicKeyHash, false)) {
        return runFinalReservationsNeighborConfirmation();
    }

    return reject("No final paths configuration was received from the coordinator. Rejected.");
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runFinalPathConfigurationCoordinatorConfirmation()
{
    debug() << "runFinalPathConfigurationCoordinatorConfirmation";
    const auto kMessage = popNextMessage<FinalPathCycleConfigurationMessage>();

    debug() << "Final payment path configuration received";

    // path was cancelled, drop all reservations belong it
    if (kMessage->amount() == 0) {
        debug() << "Final payment equals 0, transaction cancelled.";
        rollBack();
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
        return reject("Reservations on node are invalid");
    }
    info() << "All reservations are correct";

    mPaymentNodesIds = kMessage->paymentNodesIds();
    // todo check if current node is present in mPaymentNodesIds
    if (!checkAllNeighborsWithReservationsAreInFinalParticipantsList()) {
        sendMessage<FinalAmountsConfigurationResponseMessage>(
            kMessage->senderUUID,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Rejected);
        return reject("Node has reservation with participant, which not included in mPaymentNodesIds. Rejected");
    }

    auto coordinatorTotalIncomingReservationAmount = totalReservedIncomingAmountToNode(
        kMessage->senderUUID);

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (kMessage->isReceiptContains()) {
        info() << "Coordinator also send receipt";
        auto keyChain = mKeysStore->keychain(
            mTrustLines->trustLineID(kMessage->senderUUID));
        auto serializedIncomingReceiptData = getSerializedReceipt(
            kMessage->senderUUID,
            mNodeUUID,
            coordinatorTotalIncomingReservationAmount);
        if (!keyChain.checkSign(
            ioTransaction,
            serializedIncomingReceiptData.first,
            serializedIncomingReceiptData.second,
            kMessage->signature(),
            kMessage->publicKeyNumber())) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            return reject("Coordinator send invalid receipt signature. Rejected");
        }
        if (!keyChain.saveIncomingPaymentReceipt(
            ioTransaction,
            mTrustLines->auditNumber(kMessage->senderUUID),
            mTransactionUUID,
            kMessage->publicKeyNumber(),
            coordinatorTotalIncomingReservationAmount,
            kMessage->signature())) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            return reject("Can't save coordinator receipt. Rejected.");
        }
        info() << "Coordinator's receipt is valid";
    } else {
        if (coordinatorTotalIncomingReservationAmount != TrustLine::kZeroAmount()) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            warning() << "Receipt amount: 0. Local reserved incoming amount: "
                      << coordinatorTotalIncomingReservationAmount;
            return reject("Coordinator send invalid receipt amount. Rejected");
        }
    }

    mPublicKey = mKeysStore->generateAndSaveKeyPairForPaymentTransaction(
        ioTransaction,
        currentTransactionUUID(),
        mNodeUUID);
    mParticipantsPublicKeysHashes.insert(
        make_pair(
            currentNodeUUID(),
            make_pair(
                mPaymentNodesIds[mNodeUUID],
                mPublicKey->hash())));

    // send messages to all participants except coordinator:
    // to nodes with outgoing reservations - outgoing receipts and public key hash;
    // to rest nodes - only public key hash
    vector<pair<PathID, AmountReservation::ConstShared>> emptyReservations;
    for (const auto &nodeAndPaymentID : mPaymentNodesIds) {
        if (nodeAndPaymentID.first == mNodeUUID) {
            continue;
        }
        if (mReservations.find(nodeAndPaymentID.first) == mReservations.end()) {
            if (nodeAndPaymentID.first == mCoordinator) {
                continue;
            }
            sendMessage<TransactionPublicKeyHashMessage>(
                nodeAndPaymentID.first,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                mPaymentNodesIds[mNodeUUID],
                mPublicKey->hash());
            continue;
        }
        auto nodeReservations = mReservations[nodeAndPaymentID.first];
        if (nodeReservations.begin()->second->direction() == AmountReservation::Outgoing) {
            // in cycles nodes should have only one outgoing reservation
            auto outgoingReservedAmount = nodeReservations.begin()->second->amount();
            auto keyChain = mKeysStore->keychain(
                mTrustLines->trustLineID(nodeAndPaymentID.first));
            auto serializedOutgoingReceiptData = getSerializedReceipt(
                mNodeUUID,
                nodeAndPaymentID.first,
                outgoingReservedAmount);
            auto signatureAndKeyNumber = keyChain.sign(
                ioTransaction,
                serializedOutgoingReceiptData.first,
                serializedOutgoingReceiptData.second);
            if (!keyChain.saveOutgoingPaymentReceipt(
                ioTransaction,
                mTrustLines->auditNumber(nodeAndPaymentID.first),
                mTransactionUUID,
                signatureAndKeyNumber.second,
                outgoingReservedAmount,
                signatureAndKeyNumber.first)) {
                sendMessage<FinalAmountsConfigurationResponseMessage>(
                    kMessage->senderUUID,
                    mEquivalent,
                    currentNodeUUID(),
                    currentTransactionUUID(),
                    FinalAmountsConfigurationResponseMessage::Rejected);
                return reject("Can't save outgoing receipt. Rejected.");
            }
            sendMessage<TransactionPublicKeyHashMessage>(
                nodeAndPaymentID.first,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                mPaymentNodesIds[mNodeUUID],
                mPublicKey->hash(),
                signatureAndKeyNumber.second,
                signatureAndKeyNumber.first);
        } else {
            if (nodeAndPaymentID.first == mCoordinator) {
                continue;
            }
            sendMessage<TransactionPublicKeyHashMessage>(
                nodeAndPaymentID.first,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                mPaymentNodesIds[mNodeUUID],
                mPublicKey->hash());
        }
    }

    // coordinator don't send public key hash
    if (mPaymentNodesIds.size() == mParticipantsPublicKeysHashes.size() + 1) {
        // all neighbors sent theirs reservations
        if (!checkAllPublicKeyHashesProperly()) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            return reject("Public key hashes are not properly. Rejected");
        }

        sendMessage<FinalAmountsConfigurationResponseMessage>(
            kMessage->senderUUID,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Accepted,
            mPublicKey);

        info() << "Accepted final amounts configuration";

        mStep = Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsPublicKeys,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(5)); // todo : need discuss this parameter (5)
    }

    return resultWaitForMessageTypes(
        {Message::Payments_TransactionPublicKeyHash,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runFinalReservationsNeighborConfirmation()
{
    debug() << "runFinalReservationsNeighborConfirmation";
    auto kMessage = popNextMessage<TransactionPublicKeyHashMessage>();
    debug() << "sender: " << kMessage->senderUUID;

    mParticipantsPublicKeysHashes[kMessage->senderUUID] = make_pair(
        kMessage->paymentNodeID(),
        kMessage->transactionPublicKeyHash());

    auto participantTotalIncomingReservationAmount = totalReservedIncomingAmountToNode(
        kMessage->senderUUID);

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (kMessage->isReceiptContains()) {
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
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
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
    } else {
        if (participantTotalIncomingReservationAmount != TrustLine::kZeroAmount()) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            warning() << "Receipt amount: 0. Local reserved incoming amount: " << participantTotalIncomingReservationAmount;
            return reject("Sender send invalid receipt amount. Rejected");
        }
    }

    // if coordinator didn't sent final payment configuration yet
    if (mPaymentNodesIds.empty()) {
        return resultWaitForMessageTypes(
            {Message::Payments_FinalPathCycleConfiguration,
             Message::Payments_TransactionPublicKeyHash,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(1));
    }

    // coordinator already sent final amounts configuration
    // coordinator don't send public key hash
    if (mPaymentNodesIds.size() == mParticipantsPublicKeysHashes.size() + 1) {
        // all neighbors sent theirs reservations
        if (!checkAllPublicKeyHashesProperly()) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                mCoordinator,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            return reject("Public key hashes are not properly. Rejected");
        }

        sendMessage<FinalAmountsConfigurationResponseMessage>(
            mCoordinator,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Accepted,
            mPublicKey);

        info() << "Accepted final amounts configuration";

        mStep = Common_VotesChecking;
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
        return resultDone();
    }

    if (!contextIsValid(Message::Payments_ParticipantsPublicKeys) or
            contextIsValid(Message::Payments_ParticipantsVotes, false)) {
        if (mTransactionIsVoted) {
            return recover("No participants votes message with all votes received.");
        } else {
            return reject("No participants votes message received. Transaction was closed. Rolling Back");
        }
    }

    return runVotesCheckingStage();
}

const NodeUUID& CycleCloserIntermediateNodeTransaction::coordinatorUUID() const
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
    ioTransaction->historyStorage()->savePaymentRecord(
        make_shared<PaymentRecord>(
            currentTransactionUUID(),
            PaymentRecord::PaymentOperationType::CyclerCloserIntermediateType,
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

const string CycleCloserIntermediateNodeTransaction::logHeader() const
{
    stringstream s;
    s << "[CycleCloserIntermediateNodeTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
