#include "IntermediateNodePaymentTransaction.h"


IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    const NodeUUID& currentNodeUUID,
    IntermediateNodeReservationRequestMessage::ConstShared message,
    TrustLinesManager* trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log) :

    BasePaymentTransaction(
        BaseTransaction::IntermediateNodePaymentTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        trustLines,
        storageHandler,
        maxFlowCalculationCacheManager,
        log),
    mMessage(message),
    mTotalReservedAmount(0)
{
    mStep = Stages::IntermediateNode_PreviousNeighborRequestProcessing;
}

IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager* trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log) :
        BasePaymentTransaction(
            buffer,
            nodeUUID,
            trustLines,
            storageHandler,
            maxFlowCalculationCacheManager,
            log)
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

            case Stages::Common_ClarificationTransaction:
                return runClarificationOfTransaction();

            case Stages::IntermediateNode_ReservationProlongation:
                return runReservationProlongationStage();

            case Stages::Common_VotesChecking:
                return runVotesCheckingStageWithCoordinatorClarification();

            case Stages::Common_Recovery:
                return runVotesRecoveryParentStage();
            default:
                throw RuntimeError(
                    "IntermediateNodePaymentTransaction::run: "
                        "unexpected stage occurred.");
        }
    } catch (Exception &e) {
        error() << e.what();
        recover("Something happens wrong in method run(). Transaction will be recovered");
    }
}


TransactionResult::SharedConst IntermediateNodePaymentTransaction::runPreviousNeighborRequestProcessingStage()
{
    debug() << "runPreviousNeighborRequestProcessingStage";
    const auto kNeighbor = mMessage->senderUUID;
    debug() << "Init. intermediate payment operation from node (" << kNeighbor << ")";
    if (mMessage->finalAmountsConfiguration().size() == 0) {
        error() << "Not received reservation";
        // todo : add actual reaction
    }
    const auto kReservation = mMessage->finalAmountsConfiguration()[0];

    debug() << "Requested amount reservation: " << *kReservation.second.get();

    if (!mTrustLines->isNeighbor(kNeighbor)) {
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);
        error() << "Path is not valid: previous node is not neighbor of current one. Rejected.";
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        } else {
            mStep = Stages::IntermediateNode_ReservationProlongation;
            return resultWaitForMessageTypes(
                {Message::Payments_FinalAmountsConfiguration,
                 Message::Payments_IntermediateNodeReservationRequest},
                maxNetworkDelay(kMaxPathLength - 2));
        }
    }

    // update local reservations during amounts from coordinator
    if (!updateReservations(vector<pair<PathUUID, ConstSharedTrustLineAmount>>(
        mMessage->finalAmountsConfiguration().begin() + 1,
        mMessage->finalAmountsConfiguration().end()))) {
        error() << "Coordinator send path configuration, which is absent on current node";
        // todo : implement correct action
    }

    const auto kIncomingAmount = mTrustLines->availableIncomingAmount(kNeighbor);
    TrustLineAmount kReservationAmount =
            min(*kReservation.second.get(), *kIncomingAmount);

    if (0 == kReservationAmount || ! reserveIncomingAmount(kNeighbor, kReservationAmount, kReservation.first)) {
        debug() << "No amount reservation is possible.";
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        } else {
            mStep = Stages::IntermediateNode_ReservationProlongation;
            return resultWaitForMessageTypes(
                {Message::Payments_FinalAmountsConfiguration,
                 Message::Payments_IntermediateNodeReservationRequest},
                maxNetworkDelay(kMaxPathLength - 2));
        }
    }

    debug() << "reserve locally " << kReservationAmount << " to node " << kNeighbor << " on path " << kReservation.first;
    mLastReservedAmount = kReservationAmount;
    sendMessage<IntermediateNodeReservationResponseMessage>(
        kNeighbor,
        currentNodeUUID(),
        currentTransactionUUID(),
        kReservation.first,
        ResponseMessage::Accepted,
        kReservationAmount);

    mStep = Stages::IntermediateNode_CoordinatorRequestProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationRequest,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_FinalAmountsConfiguration},
        maxNetworkDelay(3));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runCoordinatorRequestProcessingStage()
{
    debug() << "runCoordinatorRequestProcessingStage";

    // If IntermediateNodeReservationResponseMessage wasn't deliver on previous stage,
    // we can receive IntermediateNodeReservationRequestMessage or FinalAmountsConfigurationMessage
    // and we should process them properly
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        return runReservationProlongationStage();
    }
    if (contextIsValid(Message::Payments_FinalAmountsConfiguration, false)) {
        return runReservationProlongationStage();
    }

    if (! contextIsValid(Message::Payments_CoordinatorReservationRequest))
        return reject("No coordinator request received. Rolled back.");


    debug() << "Coordinator further reservation request received.";


    // TODO: add check for previous nodes amount reservation


    const auto kMessage = popNextMessage<CoordinatorReservationRequestMessage>();
    const auto kNextNode = kMessage->nextNodeInPathUUID();
    if (kMessage->finalAmountsConfiguration().size() == 0) {
        error() << "Not received reservation";
        // todo : implement actual reaction
    }
    const auto kReservation = kMessage->finalAmountsConfiguration()[0];
    mCoordinator = kMessage->senderUUID;
    mLastProcessedPath = kReservation.first;

    if (!mTrustLines->isNeighbor(kNextNode)) {
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);
        debug() << "Path is not valid: next node is not neighbor of current one. Rolled back.";
        rollBack(kReservation.first);
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    // update local reservations during amounts from coordinator
    if (!updateReservations(mMessage->finalAmountsConfiguration())) {
        error() << "Coordinator send path configuration, which is absent on current node";
        // todo : implement correct action
    }

    // Note: copy of shared pointer is required
    const auto kOutgoingAmount = mTrustLines->availableOutgoingAmount(kNextNode);
    debug() << "requested reservation amount is " << *kReservation.second.get();
    debug() << "available outgoing amount to " << kNextNode << " is " << *kOutgoingAmount.get();
    TrustLineAmount reservationAmount = min(
        *kReservation.second.get(),
        *kOutgoingAmount);

    if (0 == reservationAmount || ! reserveOutgoingAmount(kNextNode, reservationAmount, kReservation.first)) {
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);

        debug() << "No amount reservation is possible. Rolled back.";
        rollBack(kReservation.first);
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    debug() << "Reserve locally " << reservationAmount << " to node " << kNextNode << " on path " << kReservation.first;
    mLastReservedAmount = reservationAmount;

    vector<pair<PathUUID, ConstSharedTrustLineAmount>> reservations;
    reservations.push_back(
        make_pair(
            kReservation.first,
            make_shared<const TrustLineAmount>(reservationAmount)));

    if (kMessage->finalAmountsConfiguration().size() > 1) {
        // add actual reservations for next node
        reservations.insert(
            reservations.end(),
            kMessage->finalAmountsConfiguration().begin() + 1,
            kMessage->finalAmountsConfiguration().end());
    }

    debug() << "send reservations size: " << reservations.size();
    sendMessage<IntermediateNodeReservationRequestMessage>(
        kNextNode,
        currentNodeUUID(),
        currentTransactionUUID(),
        reservations);

    clearContext();
    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runNextNeighborResponseProcessingStage()
{
    debug() << "runNextNeighborResponseProcessingStage";
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationResponse)) {
        debug() << "No valid amount reservation response received. Rolled back.";
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            mLastProcessedPath,
            ResponseMessage::Rejected);
        rollBack(mLastProcessedPath);
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    const auto kMessage = popNextMessage<IntermediateNodeReservationResponseMessage>();
    const auto kContractor = kMessage->senderUUID;

    if (kMessage->state() == IntermediateNodeReservationResponseMessage::Closed) {
        // Receiver reject reservation and Coordinator should close transaction
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            kMessage->pathUUID(),
            ResponseMessage::Closed);
        debug() << "Amount reservation rejected with further transaction closing by the Receiver node.";
        rollBack(kMessage->pathUUID());
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    if (kMessage->state() == IntermediateNodeReservationResponseMessage::Rejected){
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            kMessage->pathUUID(),
            ResponseMessage::Rejected);
        debug() << "Amount reservation rejected by the neighbor node.";
        rollBack(kMessage->pathUUID());
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
        mStep = Stages::IntermediateNode_ReservationProlongation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay((kMaxPathLength - 2) * 4));
    }

    debug() << "(" << kContractor << ") accepted amount reservation.";

    mLastReservedAmount = kMessage->amountReserved();
    // shortage local reservation on current path
    for (const auto &nodeReservation : mReservations) {
        for (const auto &pathUUIDAndreservation : nodeReservation.second) {
            if (pathUUIDAndreservation.first == kMessage->pathUUID()) {
                shortageReservation(
                    nodeReservation.first,
                    pathUUIDAndreservation.second,
                    mLastReservedAmount,
                    pathUUIDAndreservation.first);
            }
        }
    }

    sendMessage<CoordinatorReservationResponseMessage>(
        mCoordinator,
        currentNodeUUID(),
        currentTransactionUUID(),
        kMessage->pathUUID(),
        ResponseMessage::Accepted,
        mLastReservedAmount);

    mStep = Stages::Common_FinalPathConfigurationChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalPathConfiguration,
         Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_FinalAmountsConfiguration},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runFinalPathConfigurationProcessingStage()
{
    // receive final amount on current path
    debug() << "runFinalPathConfigurationProcessingStage";
    // if FinalPathConfigurationMessage wasn't deleivered, then according to algorithm
    // we can receive IntermediateNodeReservationRequest or FinalAmountsConfiguration
    // and we should process it properly
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        return runPreviousNeighborRequestProcessingStage();
    }
    if (contextIsValid(Message::Payments_FinalAmountsConfiguration, false)) {
        return runFinalAmountsConfigurationConfirmation();
    }

    if (! contextIsValid(Message::Payments_FinalPathConfiguration))
        return reject("No final paths configuration was received from the coordinator. Rejected.");


    const auto kMessage = popNextMessage<FinalPathConfigurationMessage>();

    // TODO: check if message was really received from the coordinator;


    debug() << "Final payment path configuration received";

    // path was cancelled, drop all reservations belong it
    if (kMessage->amount() == 0) {
        rollBack(kMessage->pathUUID());
        // if no reservations close transaction
        if (mReservations.size() == 0) {
            debug() << "There are no reservations. Transaction closed.";
            return resultDone();
        }
    } else {

        mTotalReservedAmount += kMessage->amount();
        // Shortening all reservations that belongs to this node and path.
        for (const auto &nodeAndReservations : mReservations) {
            for (const auto &pathUUIDAndReservation : nodeAndReservations.second) {
                if (pathUUIDAndReservation.first == kMessage->pathUUID()) {
                    shortageReservation(
                        nodeAndReservations.first,
                        pathUUIDAndReservation.second,
                        kMessage->amount(),
                        pathUUIDAndReservation.first);
                }
            }
        }
    }

    mStep = Stages::IntermediateNode_ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfiguration,
         Message::Payments_IntermediateNodeReservationRequest},
        maxNetworkDelay(kMaxPathLength - 2));
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
    // Node is clarifying of coordinator if transaction is still alive
    if (!contextIsValid(Message::Payments_FinalAmountsConfiguration)) {
        debug() << "Send TTLTransaction message to coordinator " << mCoordinator;
        sendMessage<TTLPolongationMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID());
        mStep = Stages::Common_ClarificationTransaction;
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
            Message::Payments_FinalAmountsConfiguration,
            Message::Payments_TTLProlongation},
            maxNetworkDelay(2));
    }
    return runFinalAmountsConfigurationConfirmation();
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runClarificationOfTransaction()
{
    // on this stage we can receive IntermediateNodeReservationRequest, FinalAmountsConfiguration
    // and ParticipantsVotes messages and on this cases we process it properly
    debug() << "runClarificationOfTransaction";
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        mMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();
        return runPreviousNeighborRequestProcessingStage();
    }
    if (contextIsValid(Message::Payments_FinalAmountsConfiguration, false)) {
        return runFinalAmountsConfigurationConfirmation();
    }
    if (contextIsValid(Message::MessageType::Payments_ParticipantsVotes, false)) {
        mStep = Stages::Common_VotesChecking;
        return runVotesCheckingStage();
    }
    if (!contextIsValid(Message::MessageType::Payments_TTLProlongation)) {
        if (mTransactionIsVoted) {
            return recover("No participants votes message with all votes received.");
        } else {
            return reject("No participants votes message received. Transaction was closed. Rolling Back");
        }
    }
    // transactions is still alive and we continue waiting for messages
    clearContext();
    debug() << "Transactions is still alive. Continue waiting for messages";
    mStep = Stages::IntermediateNode_ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes,
         Message::Payments_FinalAmountsConfiguration,
         Message::Payments_IntermediateNodeReservationRequest},
        maxNetworkDelay((kMaxPathLength - 2) * 4));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runFinalAmountsConfigurationConfirmation()
{
    // receive final configuration on all paths
    debug() << "runFinalAmountsConfigurationConfirmation";

    auto kMessage = popNextMessage<FinalAmountsConfigurationMessage>();
    if (!updateReservations(
        kMessage->finalAmountsConfiguration())) {
        sendMessage<FinalAmountsConfigurationResponseMessage>(
            kMessage->senderUUID,
            currentNodeUUID(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Rejected);
        return reject("There are some final amounts, reservations for which are absent. Rejected");
    }

    debug() << "All reservations was updated";
    sendMessage<FinalAmountsConfigurationResponseMessage>(
        kMessage->senderUUID,
        currentNodeUUID(),
        currentTransactionUUID(),
        FinalAmountsConfigurationResponseMessage::Accepted);

    mStep = Common_VotesChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxNetworkDelay(3));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runVotesCheckingStageWithCoordinatorClarification()
{
    if (contextIsValid(Message::Payments_ParticipantsVotes, false)) {
        return runVotesCheckingStage();
    }
    debug() << "Send TTLTransaction message to coordinator " << mCoordinator;
    sendMessage<TTLPolongationMessage>(
        mCoordinator,
        currentNodeUUID(),
        currentTransactionUUID());
    mStep = Stages::Common_ClarificationTransaction;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes,
         Message::Payments_TTLProlongation},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::approve()
{
    BasePaymentTransaction::approve();
    runBuildFourNodesCyclesSignal();
    runBuildThreeNodesCyclesSignal();
    return resultDone();
}

void IntermediateNodePaymentTransaction::runBuildFourNodesCyclesSignal()
{
    vector<pair<NodeUUID, NodeUUID>> debtorsAndCreditorsFourCycles;
    map<PathUUID, NodeUUID> pathsReservations;
    for (const auto &itNodeUUIDAndReservations : mReservations) {
        for (const auto &itPathUUIDAndReservation : itNodeUUIDAndReservations.second) {
            auto pathReservation = pathsReservations.find(itPathUUIDAndReservation.first);
            if (pathReservation == pathsReservations.end()) {
                pathsReservations.insert(
                    make_pair(
                        itPathUUIDAndReservation.first,
                        itNodeUUIDAndReservations.first));
            } else {
                if (itPathUUIDAndReservation.second->direction() == AmountReservation::Outgoing) {
                    debtorsAndCreditorsFourCycles.push_back(
                        make_pair(
                            pathReservation->second,
                            itNodeUUIDAndReservations.first));
                } else if (itPathUUIDAndReservation.second->direction() == AmountReservation::Incoming) {
                    debtorsAndCreditorsFourCycles.push_back(
                        make_pair(
                            itNodeUUIDAndReservations.first,
                            pathReservation->second));
                }
            }
        }
    }
    mBuildCycleFourNodesSignal(
        debtorsAndCreditorsFourCycles);
}

void IntermediateNodePaymentTransaction::runBuildThreeNodesCyclesSignal()
{
    vector<NodeUUID> contractorsUUID;
    contractorsUUID.reserve(mReservations.size());
    for (auto const nodeUUIDAndReservations : mReservations) {
        contractorsUUID.push_back(
            nodeUUIDAndReservations.first);
    }
    mBuildCycleThreeNodesSignal(
        contractorsUUID);
}

void IntermediateNodePaymentTransaction::savePaymentOperationIntoHistory()
{
    debug() << "savePaymentOperationIntoHistory";
    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->historyStorage()->savePaymentRecord(
        make_shared<PaymentRecord>(
            currentTransactionUUID(),
            PaymentRecord::PaymentOperationType::IntermediatePaymentType,
            mTotalReservedAmount));
}

bool IntermediateNodePaymentTransaction::checkReservationsDirections() const
{
    debug() << "checkReservationsDirections";
    for (const auto nodeUUIDAndReservations : mReservations) {
        for (const auto pathUUIDAndReservation : nodeUUIDAndReservations.second) {
            const auto checkedPath = pathUUIDAndReservation.first;
            const auto checkedAmount = pathUUIDAndReservation.second->amount();
            int countIncomingReservations = 0;
            int countOutgoingReservations = 0;

            for (const auto nodeUUIDAndReservationsInternal : mReservations) {
                for (const auto pathUUIDAndReservationInternal : nodeUUIDAndReservationsInternal.second) {
                    if (pathUUIDAndReservationInternal.first == checkedPath) {
                        if (pathUUIDAndReservationInternal.second->amount() != checkedAmount) {
                            error() << "Amounts are different on path " << checkedPath;
                            return false;
                        }
                        if (pathUUIDAndReservationInternal.second->direction() == AmountReservation::Outgoing) {
                            countOutgoingReservations++;
                        }
                        if (pathUUIDAndReservationInternal.second->direction() == AmountReservation::Incoming) {
                            countIncomingReservations++;
                        }
                    }
                }
            }

            if (countIncomingReservations != 1 || countOutgoingReservations != 1) {
                error() << "Count incoming and outgoing reservations are invalid on path " << checkedPath;
                return false;
            }
        }
    }
    return true;
}

const string IntermediateNodePaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[IntermediateNodePaymentTA: " << currentTransactionUUID() << "] ";
    return s.str();
}