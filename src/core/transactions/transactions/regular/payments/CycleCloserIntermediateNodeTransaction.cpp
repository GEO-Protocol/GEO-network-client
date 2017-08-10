#include "CycleCloserIntermediateNodeTransaction.h"

CycleCloserIntermediateNodeTransaction::CycleCloserIntermediateNodeTransaction(
    const NodeUUID& currentNodeUUID,
    IntermediateNodeCycleReservationRequestMessage::ConstShared message,
    TrustLinesManager* trustLines,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log,
    TestingController *testingController) :

    BasePaymentTransaction(
        BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log,
        testingController),
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
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log,
    TestingController *testingController) :

    BasePaymentTransaction(
        buffer,
        nodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log,
        testingController),
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

            case Stages::IntermediateNode_ReservationProlongation:
                return runReservationProlongationStage();

            case Stages::Common_VotesChecking:
                return runVotesCheckingStage();

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
        error() << e.what();
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

    if (!mTrustLines->isNeighbor(mPreviousNode)) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            mPreviousNode,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);
        error() << "Path is not valid: previous node is not neighbor of current one. Rejected.";
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
                currentNodeUUID(),
                currentTransactionUUID(),
                ResponseCycleMessage::Rejected);
            debug() << "can't reserve requested amount, event with reservations, transaction closed";
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
        for (auto reservation : reservations) {
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
                return resultAwaikAfterMilliseconds(
                    kWaitingForReleasingAmountMSec);
            }
            debug() << "don't win reservation";
        }
    }

    if (0 == mReservationAmount || ! reserveIncomingAmount(mPreviousNode, mReservationAmount, 0)) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            mPreviousNode,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);
        debug() << "can't reserve requested amount, transaction closed";
        return resultDone();
    }

    mLastReservedAmount = mReservationAmount;
    debug() << "send accepted message with reserve (" << mReservationAmount << ") to node " << mPreviousNode;
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        currentNodeUUID(),
        currentTransactionUUID(),
        ResponseCycleMessage::Accepted,
        mReservationAmount);

    mStep = Stages::IntermediateNode_CoordinatorRequestProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationRequest},
        maxNetworkDelay(3));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runPreviousNeighborRequestProcessingStageAgain()
{
    debug() << "runPreviousNeighborRequestProcessingStageAgain";
    if (mCyclesManager->isTransactionStillAlive(
        mConflictedTransaction)) {
        debug() << "wait again";
        return resultAwaikAfterMilliseconds(
            kWaitingForReleasingAmountMSec);
    }
    debug() << "try reserve " << mReservationAmount;
    if (! reserveIncomingAmount(mPreviousNode, mReservationAmount, 0)) {
        sendMessage<IntermediateNodeCycleReservationResponseMessage>(
            mPreviousNode,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);
        debug() << "can't reserve requested amount, transaction closed";
        return resultDone();
    }

    mLastReservedAmount = mReservationAmount;
    debug() << "send accepted message with reserve (" << mReservationAmount << ") to node " << mPreviousNode;
    sendMessage<IntermediateNodeCycleReservationResponseMessage>(
        mPreviousNode,
        currentNodeUUID(),
        currentTransactionUUID(),
        ResponseCycleMessage::Accepted,
        mReservationAmount);

    mStep = Stages::IntermediateNode_CoordinatorRequestProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorCycleReservationRequest},
        maxNetworkDelay(3));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runCoordinatorRequestProcessingStage()
{
    debug() << "runCoordinatorRequestProcessingStage";
    if (! contextIsValid(Message::Payments_CoordinatorCycleReservationRequest))
        return reject("No coordinator request received. Rolled back.");

    debug() << "Coordinator further reservation request received.";

    // TODO: add check for previous nodes amount reservation

    const auto kMessage = popNextMessage<CoordinatorCycleReservationRequestMessage>();
    mNextNode = kMessage->nextNodeInPathUUID();

    debug() << "Requested amount reservation: " << kMessage->amount();
    debug() << "Next node is " << kMessage->nextNodeInPathUUID();

    if (!mTrustLines->isNeighbor(mNextNode)) {
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);

        error() << "Path is not valid: next node is not neighbor of current one. Rejected.";
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
                currentNodeUUID(),
                currentTransactionUUID(),
                ResponseCycleMessage::Rejected);

            debug() << "No amount reservation is possible, even with using other reservations. Rolled back.";
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
        for (auto reservation : reservations) {
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
                return resultAwaikAfterMilliseconds(
                    kWaitingForReleasingAmountMSec);
            }
            debug() << "don't win reservation";
        }
    }
    if (0 == mReservationAmount || ! reserveOutgoingAmount(mNextNode, mReservationAmount, 0)) {
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);

        debug() << "No amount reservation is possible. Rolled back.";
        rollBack();
        return resultDone();
    }

    mLastReservedAmount = mReservationAmount;
    debug() << "Send request reservation on " << mReservationAmount << " to " << mNextNode;
    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        mNextNode,
        currentNodeUUID(),
        currentTransactionUUID(),
        mReservationAmount,
        mCoordinator,
        mCycleLength);

    clearContext();
    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runCoordinatorRequestProcessingStageAgain()
{
    debug() << "runCoordinatorRequestProcessingStageAgain";
    if (mCyclesManager->isTransactionStillAlive(
        mConflictedTransaction)) {
        debug() << "wait again";
        return resultAwaikAfterMilliseconds(
            kWaitingForReleasingAmountMSec);
    }
    debug() << "try reserve " << mReservationAmount;
    if (! reserveOutgoingAmount(mNextNode, mReservationAmount, 0)) {
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);

        debug() << "No amount reservation is possible. Rolled back.";
        rollBack();
        return resultDone();
    }

    mLastReservedAmount = mReservationAmount;
    sendMessage<IntermediateNodeCycleReservationRequestMessage>(
        mNextNode,
        currentNodeUUID(),
        currentTransactionUUID(),
        mReservationAmount,
        mCoordinator,
        mCycleLength);

    clearContext();
    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeCycleReservationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runNextNeighborResponseProcessingStage()
{
    debug() << "runNextNeighborResponseProcessingStage";
    if (! contextIsValid(Message::Payments_IntermediateNodeCycleReservationResponse)) {
        debug() << "No valid amount reservation response received. Rolled back.";
        rollBack();
        return resultDone();
    }

    const auto kMessage = popNextMessage<IntermediateNodeCycleReservationResponseMessage>();
    const auto kContractor = kMessage->senderUUID;

    if (kMessage->state() == IntermediateNodeCycleReservationResponseMessage::Rejected){
        sendMessage<CoordinatorCycleReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseCycleMessage::Rejected);
        debug() << "Amount reservation rejected by the neighbor node.";
        rollBack();
        return resultDone();
    }

    debug() << "(" << kContractor << ") accepted amount reservation.";
    mLastReservedAmount = kMessage->amountReserved();
    // shortage local reservation on current path
    for (const auto &nodeReservation : mReservations) {
        for (const auto &pathUUIDAndreservation : nodeReservation.second) {
            shortageReservation(
                nodeReservation.first,
                pathUUIDAndreservation.second,
                mLastReservedAmount,
                pathUUIDAndreservation.first);
        }
    }

    debug() << "send accepted message with reserve (" << mLastReservedAmount << ") to node " << mCoordinator;
    sendMessage<CoordinatorCycleReservationResponseMessage>(
        mCoordinator,
        currentNodeUUID(),
        currentTransactionUUID(),
        ResponseCycleMessage::Accepted,
        mLastReservedAmount);

    mStep = Stages::Common_FinalPathConfigurationChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalPathCycleConfiguration},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runReservationProlongationStage()
{
    debug() << "runReservationProlongationStage";
    // In case if participants votes message is already received -
    // there is no need to prolong reservation, transaction may be proceeded.
    // Node is clarifying of coordinator if transaction is still alive
    if (!contextIsValid(Message::Payments_ParticipantsVotes)) {
        return reject("No participants votes message received. Transaction was closed. Rolling Back");
    }
    mStep = Stages::Common_VotesChecking;
    return runVotesCheckingStage();
}

TransactionResult::SharedConst CycleCloserIntermediateNodeTransaction::runFinalPathConfigurationProcessingStage()
{
    debug() << "runFinalPathConfigurationProcessingStage";
    if (! contextIsValid(Message::Payments_FinalPathCycleConfiguration))
        return reject("No final paths configuration was received from the coordinator. Rejected.");


    const auto kMessage = popNextMessage<FinalPathCycleConfigurationMessage>();

    // TODO: check if message was really received from the coordinator;


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
        for (const auto &pathUUIDAndReservation : nodeAndReservations.second) {
            shortageReservation(
                nodeAndReservations.first,
                pathUUIDAndReservation.second,
                kMessage->amount(),
                pathUUIDAndReservation.first);
        }
    }

    debug() << "Final payment path was updated";
    sendMessage<FinalAmountsConfigurationResponseMessage>(
        kMessage->senderUUID,
        currentNodeUUID(),
        currentTransactionUUID(),
        FinalAmountsConfigurationResponseMessage::Accepted);

    mStep = Stages::IntermediateNode_ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxNetworkDelay(kMaxPathLength - 2));
}

const NodeUUID& CycleCloserIntermediateNodeTransaction::coordinatorUUID() const
{
    return mCoordinator;
}

const uint8_t CycleCloserIntermediateNodeTransaction::cycleLength() const
{
    return mCycleLength;
}

void CycleCloserIntermediateNodeTransaction::savePaymentOperationIntoHistory()
{
    debug() << "savePaymentOperationIntoHistory";
    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->historyStorage()->savePaymentRecord(
        make_shared<PaymentRecord>(
            currentTransactionUUID(),
            PaymentRecord::PaymentOperationType::CyclerCloserIntermediateType,
            mLastReservedAmount));
}

bool CycleCloserIntermediateNodeTransaction::checkReservationsDirections() const
{
    debug() << "checkReservationsDirections";
    if (mReservations.size() != 2) {
        error() << "Wrong nodes reservations size: " << mReservations.size();
        return false;
    }

    auto firstNodeReservation = mReservations.begin()->second;
    auto secondNodeReservation = mReservations.end() ->second;
    if (firstNodeReservation.size() != 1 || secondNodeReservation.size() != 1) {
        error() << "Wrong reservations size";
        return false;
    }
    const auto firstReservation = firstNodeReservation.at(0);
    const auto secondReservation = secondNodeReservation.at(0);
    if (firstReservation.first != secondReservation.first) {
        error() << "Reservations on different ways";
        return false;
    }
    if (firstReservation.second->amount() != secondReservation.second->amount()) {
        error() << "Different reservations amount";
        return false;
    }
    if (firstReservation.second->direction() == secondReservation.second->direction()) {
        error() << "Wrong directions";
        return false;
    }
    return true;
}

const string CycleCloserIntermediateNodeTransaction::logHeader() const
{
    stringstream s;
    s << "[CycleCloserIntermediateNodeTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
