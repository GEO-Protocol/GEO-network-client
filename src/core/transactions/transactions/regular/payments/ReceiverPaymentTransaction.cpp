#include "ReceiverPaymentTransaction.h"


ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    const NodeUUID &currentNodeUUID,
    ReceiverInitPaymentRequestMessage::ConstShared message,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController,
    VisualInterface *visualInterface) :

    BasePaymentTransaction(
        BaseTransaction::ReceiverPaymentTransaction,
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
    mTransactionShouldBeRejected(false),
    mVisualInterface(visualInterface)
{
    mStep = Stages::Receiver_CoordinatorRequestApproving;
}

ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLines,
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
        subsystemsController)
{}

TransactionResult::SharedConst ReceiverPaymentTransaction::run()
    noexcept
{
    try {
        debug() << "mStep: " << mStep;
        switch (mStep) {
            case Stages::Receiver_CoordinatorRequestApproving:
                return runInitialisationStage();

            case Stages::Receiver_AmountReservationsProcessing:
                return runAmountReservationStage();

            case Stages::Common_ClarificationTransactionBeforeVoting:
                return runClarificationOfTransactionBeforeVoting();

            case Stages::Coordinator_FinalAmountsConfigurationConfirmation:
                return runFinalAmountsConfigurationConfirmation();

            case Stages::Common_ClarificationTransactionDuringFinalAmountsClarification:
                return runClarificationOfTransactionDuringFinalAmountsClarification();

            case Stages::Common_VotesChecking:
                return runVotesCheckingStageWithCoordinatorClarification();

            case Stages::Common_ClarificationTransactionDuringVoting:
                return runClarificationOfTransactionDuringVoting();

            case Stages::Common_Recovery:
                return runVotesRecoveryParentStage();


            default:
                throw RuntimeError(
                    "ReceiverPaymentTransaction::run(): "
                        "invalid stage number occurred");
        }
    } catch (Exception &e) {
        warning() << e.what();
        return recover("Something happens wrong in method run(). Transaction will be recovered");
    }
}

const string ReceiverPaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiverPaymentTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runInitialisationStage()
{
    const auto kCoordinator = mMessage->senderUUID;
    debug() << "Operation for " << mMessage->amount() << " initialised by the (" << kCoordinator << ")";

    // Check if total incoming possibilities of the node are <= of the payment amount.
    // If not - there is no reason to process the operation at all.
    // (reject operation)
    const auto kTotalAvailableIncomingAmount = *(mTrustLines->totalIncomingAmount());
    debug() << "Total incoming amount: " << kTotalAvailableIncomingAmount;
    if (kTotalAvailableIncomingAmount < mMessage->amount()) {
        sendMessage<ReceiverInitPaymentResponseMessage>(
            kCoordinator,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            mMessage->pathID(),
            ReceiverInitPaymentResponseMessage::Rejected);

        info() << "Operation rejected due to insufficient funds.";
        return resultDone();
    }

    sendMessage<ReceiverInitPaymentResponseMessage>(
        kCoordinator,
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID(),
        mMessage->pathID(),
        ReceiverInitPaymentResponseMessage::Accepted);

    if (mSubsystemsController->isWriteVisualResults()) {
        stringstream s;
        s << VisualResult::ReceiverOnPayment << kTokensSeparator
          << microsecondsSinceUnixEpoch() << kTokensSeparator
          << currentTransactionUUID() << kTokensSeparator
          << mMessage->senderUUID << kCommandsSeparator;
        auto message = s.str();

        try {
            mVisualInterface->writeResult(
                message.c_str(),
                message.size());
        } catch (IOError &e) {
            error() << "ReceiverPaymentTransaction::runInitialisationStage: "
                            "Error occurred when visual result has accepted. Details: " << e.message();
        }
    }

    // Begin waiting for amount reservation requests.
    // There is non-zero probability, that first couple of paths will fail.
    // So receiver will wait for time, that is approximately need for several nodes for processing.
    //
    // TODO: enhancement: send approximate paths count to receiver, so it will be able to wait correct timeout.
    mStep = Stages::Receiver_AmountReservationsProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationRequest,
        Message::Payments_TTLProlongationResponse},
        maxNetworkDelay((kMaxPathLength - 1) * 4));
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runAmountReservationStage()
{
    debug() << "runAmountReservationStage";
    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        // current path was rejected and need reset delay time
        debug() << "Receive TTL prolongation message";
        const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
        if (kMessage->state() == TTLProlongationResponseMessage::Continue) {
            debug() << "Transactions is still alive. Continue waiting for messages";
            return resultWaitForMessageTypes(
                {Message::Payments_IntermediateNodeReservationRequest,
                 Message::Payments_TTLProlongationResponse},
                maxNetworkDelay((kMaxPathLength - 1) * 4));
        }
        return reject("Coordinator send TTL message with transaction finish state. Rolling Back");
    }

    if (! contextIsValid(Message::Payments_IntermediateNodeReservationRequest)) {
        debug() << "No amount reservation request was received.";
        debug() << "Send TTLTransaction message to coordinator " << mMessage->senderUUID;
        sendMessage<TTLProlongationRequestMessage>(
            mMessage->senderUUID,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID());
        mStep = Stages::Common_ClarificationTransactionBeforeVoting;
        return resultWaitForMessageTypes(
            {Message::Payments_TTLProlongationResponse,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay(2));
    }

    const auto kMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();

    const auto kNeighbor = kMessage->senderUUID;
    if (kMessage->finalAmountsConfiguration().empty()) {
        warning() << "Reservation vector is empty";
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            0,                  // 0, because we don't know pathID
            ResponseMessage::Closed);
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 1) * 4));
    }

    const auto kReservation = kMessage->finalAmountsConfiguration()[0];
    debug() << "Amount reservation for " << *kReservation.second.get() << " request received from "
            << kNeighbor << " [" << kReservation.first << "]";
    debug() << "Received reservations size: " << kMessage->finalAmountsConfiguration().size();

    if (! mTrustLines->isNeighbor(kNeighbor)) {
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);
        warning() << "Path is not valid: previous node is not neighbor of current one. Rejected.";
        // Message was sent from node, that is not listed in neighbors list.
        //
        // TODO: enhance this check
        // Neighbor public key must be used here.

        // Ignore received message.
        // Begin waiting for another one.
        //
        // TODO: enhancement: send approximate paths count to receiver, so it will be able to wait correct timeout.
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
            Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 1) * 4));
    }

    // update local reservations during amounts from coordinator
    if (!updateReservations(vector<pair<PathID, ConstSharedTrustLineAmount>>(
        kMessage->finalAmountsConfiguration().begin() + 1,
        kMessage->finalAmountsConfiguration().end()))) {
        warning() << "Coordinator send path configuration, which is absent on current node";
        // next loop is only logger info
        for (const auto &reservation : kMessage->finalAmountsConfiguration()) {
            debug() << "path: " << reservation.first << " amount: " << *reservation.second.get();
        }
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Closed);

        mTransactionShouldBeRejected = true;
        // We should waiting for possible new messages and close transaction on timeout
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 1) * 4));
    }
    debug() << "All reservations was updated";

    // Note: copy of shared pointer is required.
    const auto kAvailableAmount = mTrustLines->incomingTrustAmountConsideringReservations(kNeighbor);
    if (*kAvailableAmount == TrustLine::kZeroAmount()) {
        warning() << "Available amount equals zero. Reservation reject.";
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);

        // Begin accepting other reservation messages
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 1) * 4));
    }

    debug() << "Available amount " << *kAvailableAmount;
    const auto kReservationAmount = min(
        *kReservation.second.get(),
        *kAvailableAmount);

#ifdef TESTS
    if (kMessage->senderUUID == mMessage->senderUUID) {
        mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
            NodeUUID::empty(),
            TrustLine::kZeroAmount());
    }
    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
        kMessage->senderUUID,
        kReservationAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    if (! reserveIncomingAmount(kNeighbor, kReservationAmount, kReservation.first)) {
        // Receiver must not confirm reservation in case if requested amount is less than available.
        // Intermediate nodes may decrease requested reservation amount, but receiver must not do this.
        // It must stay synchronised with previous node.
        // So, in case if requested amount is less than available -
        // previous node must report about it to the coordinator.
        // In this case, receiver should even not receive reservation request at all.
        //
        // Also, this kind of problem may appear when two nodes are involved
        // in several COUNTER transactions.
        // In this case, balances may be reserved on the nodes,
        // but neighbor node may reject reservation,
        // because it already reserved amount or other transactions,
        // that wasn't approved by the current node yet.
        //
        // In this case, reservation request must be rejected.
        warning() << "Can't reserve incoming amount. Reservation reject.";
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Rejected);

        // Begin accepting other reservation messages
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
            Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 1) * 4));
    }

    const auto kTotalTransactionAmount = mMessage->amount();
    const auto kTotalReserved = totalReservedAmount(
        AmountReservation::Incoming);
    if (kTotalReserved > kTotalTransactionAmount){
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            kReservation.first,
            ResponseMessage::Closed);

        mTransactionShouldBeRejected = true;
        warning() << "Reserved amount is greater than requested. It indicates protocol or realisation error.";
        // We should waiting for possible new messages and close transaction on timeout
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 1) * 4));
    }

    debug() << "Reserved locally: " << kReservationAmount;
    sendMessage<IntermediateNodeReservationResponseMessage>(
        kNeighbor,
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID(),
        kReservation.first,
        ResponseMessage::Accepted,
        kReservationAmount);

    if (kTotalReserved == mMessage->amount()) {
        // Reserved amount is enough to move to votes processing stage.

        // TODO: receiver must know, how many paths are involved into the transaction.
        // This info helps to calculate max timeout,
        // that would be used for waiting for votes list message.

        mStep = Stages::Coordinator_FinalAmountsConfigurationConfirmation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_ReservationsInRelationToNode,
             Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(kMaxPathLength - 1));

    } else {
        // Waiting for another reservation request
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 1) * 4));
    }
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runClarificationOfTransactionBeforeVoting()
{
    debug() << "runClarificationOfTransactionBeforeVoting";
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        return runAmountReservationStage();
    }

    if (!contextIsValid(Message::MessageType::Payments_TTLProlongationResponse)) {
        return reject("No participants votes message received. Transaction was closed. Rolling Back");
    }

    const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
    if (kMessage->state() == TTLProlongationResponseMessage::Continue) {
        // transactions is still alive and we continue waiting for messages
        debug() << "Transactions is still alive. Continue waiting for messages";
        mStep = Stages::Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationResponse,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 1) * 4));
    }
    return reject("Coordinator send TTL message with transaction finish state. Rolling Back");
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runFinalAmountsConfigurationConfirmation()
{
    debug() << "runFinalAmountsConfigurationConfirmation";
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        mStep = Receiver_AmountReservationsProcessing;
        return runAmountReservationStage();
    }

    if (contextIsValid(Message::Payments_FinalAmountsConfiguration, false)) {
        return runFinalReservationsCoordinatorConfirmation();
    }

    if (contextIsValid(Message::Payments_ReservationsInRelationToNode, false)) {
        return runFinalReservationsNeighborConfirmation();
    }

    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        debug() << "Receive TTL prolongation message";
        const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
        if (kMessage->state() == TTLProlongationResponseMessage::Continue) {
            debug() << "Transactions is still alive. Continue waiting for messages";
            return resultWaitForMessageTypes(
                {Message::Payments_IntermediateNodeReservationRequest,
                 Message::Payments_TTLProlongationResponse,
                 Message::Payments_FinalAmountsConfiguration,
                 Message::Payments_ReservationsInRelationToNode},
                maxNetworkDelay((kMaxPathLength - 1) * 4));
        }
        return reject("Coordinator send TTL message with transaction finish state. Rolling Back");
    }

    debug() << "Send TTLTransaction message to coordinator " << mMessage->senderUUID;
    sendMessage<TTLProlongationRequestMessage>(
        coordinatorUUID(),
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID());
    mStep = Common_ClarificationTransactionDuringFinalAmountsClarification;

    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfiguration,
         Message::Payments_ReservationsInRelationToNode,
         Message::Payments_TTLProlongationResponse,
         Message::Payments_IntermediateNodeReservationRequest},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runFinalReservationsCoordinatorConfirmation()
{
    debug() << "runFinalReservationsCoordinatorConfirmation";
#ifdef TESTS
    mSubsystemsController->testForbidSendMessageOnFinalAmountClarificationStage();
#endif

    auto kMessage = popNextMessage<FinalAmountsConfigurationMessage>();
    if (!updateReservations(
            kMessage->finalAmountsConfiguration())) {
        sendMessage<FinalAmountsConfigurationResponseMessage>(
            coordinatorUUID(),
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Rejected);
        // todo : discuss if receiver can reject TA on this stage or should wait
        return reject("There are some final amounts, reservations for which are absent. Rejected");
    }

#ifdef TESTS
    // coordinator wait for this message maxNetworkDelay(2)
    mSubsystemsController->testSleepOnFinalAmountClarificationStage(maxNetworkDelay(3));
#endif

    debug() << "All reservations was updated";
    if (!checkReservationsDirections()) {
        return reject("Reservations on node are invalid");
    }
    info() << "All reservations directions are correct";

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
        info() << "Coordinator also send receipt " << kMessage->amount();

        if (coordinatorTotalIncomingReservationAmount != kMessage->amount()) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            warning() << "Receipt amount: " << kMessage->amount()
                      << ". Local incoming amount: " << coordinatorTotalIncomingReservationAmount;
            return reject("Coordinator send invalid receipt amount. Rejected");
        }

        auto keyChain = mKeysStore->keychain(
            mTrustLines->trustLineReadOnly(kMessage->senderUUID)->trustLineID());
        // todo understand what data should be for check signature
        BytesShared someData;
        if (!keyChain.checkSign(
            ioTransaction,
            someData,
            4,
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
        mNeighborsIncomingReceipts.insert(
            make_pair(
                kMessage->senderUUID,
                make_pair(
                    kMessage->signature(),
                    kMessage->publicKeyNumber())));
        info() << "Coordinator's receipt is valid";
    } else {
        if (coordinatorTotalIncomingReservationAmount != TrustLine::kZeroAmount()) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            warning() << "Receipt amount: 0. Local incoming amount: " << coordinatorTotalIncomingReservationAmount;
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

    // send public key hash to all participants except coordinator
    for (const auto &nodeAndPaymentID : mPaymentNodesIds) {
        if (nodeAndPaymentID.first == coordinatorUUID()) {
            continue;
        }
        if (nodeAndPaymentID.first == mNodeUUID) {
            continue;
        }
        info() << "Send public key hash to " << nodeAndPaymentID.first;
        sendMessage<TransactionPublicKeyHashMessage>(
            nodeAndPaymentID.first,
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            mPaymentNodesIds[mNodeUUID],
            mPublicKey->hash());
    }

    // coordinator don't send public key hash
    if (mPaymentNodesIds.size() == mParticipantsPublicKeysHashes.size() + 1) {
        info() << "All neighbors send theirs reservations";
        // all neighbors sent theirs reservations
        if (!checkAllPublicKeyHashesProperly()) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                coordinatorUUID(),
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            return reject("Public key hashes are not properly. Rejected");
        }
        info() << "All public key hashes are properly";

        sendMessage<FinalAmountsConfigurationResponseMessage>(
            coordinatorUUID(),
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Accepted,
            mPublicKey);
        info() << "Accepted final amounts configuration";

        mStep = Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(5)); // todo : need discuss this parameter (5)
    }

    return resultWaitForMessageTypes(
        {Message::Payments_ReservationsInRelationToNode,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runFinalReservationsNeighborConfirmation()
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
        info() << "Sender also send receipt " << kMessage->amount();
        if (participantTotalIncomingReservationAmount != kMessage->amount()) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            warning() << "Local reserved incoming amount: " << participantTotalIncomingReservationAmount;
            return reject("Sender send invalid receipt amount. Rejected");
        }

        auto keyChain = mKeysStore->keychain(
            mTrustLines->trustLineReadOnly(kMessage->senderUUID)->trustLineID());
        // todo understand what data should be for check signature
        BytesShared someData;
        if (!keyChain.checkSign(
            ioTransaction,
            someData,
            4,
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
        mNeighborsIncomingReceipts.insert(
            make_pair(
                kMessage->senderUUID,
                make_pair(
                    kMessage->signature(),
                    kMessage->publicKeyNumber())));
        info() << "Sender's receipt is valid";
    } else {
        if (participantTotalIncomingReservationAmount != TrustLine::kZeroAmount()) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                kMessage->senderUUID,
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            warning() << "Receipt amount: 0. Local reserved incoming amount: "
                      << participantTotalIncomingReservationAmount;
            return reject("Sender send invalid receipt amount. Rejected");
        }
    }

    // if coordinator didn't sent final payment configuration yet
    if (mPaymentNodesIds.empty()) {
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_ReservationsInRelationToNode,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(1));
    }

    // coordinator already sent final amounts configuration
    // coordinator don't send public key hash
    if (mPaymentNodesIds.size() == mParticipantsPublicKeysHashes.size() + 1) {
        // all neighbors sent theirs reservations
        if (!checkAllPublicKeyHashesProperly()) {
            sendMessage<FinalAmountsConfigurationResponseMessage>(
                coordinatorUUID(),
                mEquivalent,
                currentNodeUUID(),
                currentTransactionUUID(),
                FinalAmountsConfigurationResponseMessage::Rejected);
            return reject("Public key hashes are not properly. Rejected");
        }

        sendMessage<FinalAmountsConfigurationResponseMessage>(
            coordinatorUUID(),
            mEquivalent,
            currentNodeUUID(),
            currentTransactionUUID(),
            FinalAmountsConfigurationResponseMessage::Accepted,
            mPublicKey);
        info() << "Accepted final amounts configuration";

        mStep = Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(5)); // todo : need discuss this parameter (5)
    }

    // not all neighbors sent theirs reservations
    return resultWaitForMessageTypes(
        {Message::Payments_ReservationsInRelationToNode,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runClarificationOfTransactionDuringFinalAmountsClarification()
{
    debug() << "runClarificationOfTransactionDuringFinalAmountsClarification";
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        return runAmountReservationStage();
    }

    if (contextIsValid(Message::Payments_FinalAmountsConfiguration, false) or
            contextIsValid(Message::Payments_ReservationsInRelationToNode, false)) {
        mStep = Coordinator_FinalAmountsConfigurationConfirmation;
        return runFinalAmountsConfigurationConfirmation();
    }

    if (!contextIsValid(Message::MessageType::Payments_TTLProlongationResponse)) {
        return reject("No participants votes message received. Transaction was closed. Rolling Back");
    }

    const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
    if (kMessage->state() == TTLProlongationResponseMessage::Continue) {
        // transactions is still alive and we continue waiting for messages
        debug() << "Transactions is still alive. Continue waiting for messages";
        mStep = Stages::Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationResponse,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay((kMaxPathLength - 1) * 4));
    }
    return reject("Coordinator send TTL message with transaction finish state. Rolling Back");
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runVotesCheckingStageWithCoordinatorClarification()
{
    debug() << "runVotesCheckingStageWithCoordinatorClarification";

    if (contextIsValid(Message::Payments_TTLProlongationResponse, false)) {
        debug() << "Receive TTL prolongation message";
        const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
        if (kMessage->state() == TTLProlongationResponseMessage::Continue) {
            debug() << "Transactions is still alive. Continue waiting for messages";
            return resultWaitForMessageTypes(
                {Message::Payments_IntermediateNodeReservationRequest,
                 Message::Payments_TTLProlongationResponse,
                 Message::Payments_ParticipantsVotes},
                maxNetworkDelay((kMaxPathLength - 1) * 4));
        }
        return reject("Coordinator send TTL message with transaction finish state. Rolling Back");
    }

    if (contextIsValid(Message::Payments_ParticipantsVotes, false)) {
        if (mTransactionShouldBeRejected) {
            // this case can happens only with Receiver,
            // when coordinator wants to reserve greater then command amount
            reject("Receiver rejected transaction because of discrepancy reservations with Coordinator. Rolling back.");
        }
        return runVotesCheckingStage();
    }

    debug() << "Send TTLTransaction message to coordinator " << mMessage->senderUUID;
    sendMessage<TTLProlongationRequestMessage>(
        mMessage->senderUUID,
        mEquivalent,
        currentNodeUUID(),
        currentTransactionUUID());
    mStep = Stages::Common_ClarificationTransactionDuringVoting;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes,
         Message::Payments_TTLProlongationResponse,
         Message::Payments_IntermediateNodeReservationRequest},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runClarificationOfTransactionDuringVoting()
{
    // on this stage we can also receive and ParticipantsVotes messages
    // and on this cases we process it properly
    debug() << "runClarificationOfTransactionDuringVoting";

    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        if (mTransactionIsVoted) {
            return reject("Received IntermediateNodeReservationRequest message after voting");
        }
        mStep = Receiver_AmountReservationsProcessing;
        return runAmountReservationStage();
    }

    if (contextIsValid(Message::MessageType::Payments_ParticipantsVotes, false)) {
        mStep = Stages::Common_VotesChecking;
        return runVotesCheckingStage();
    }

    if (!contextIsValid(Message::MessageType::Payments_TTLProlongationResponse)) {
        if (mTransactionIsVoted) {
            return recover("No participants votes message with all votes received.");
        } else {
            return reject("No participants votes message received. Transaction was closed. Rolling Back");
        }
    }

    const auto kMessage = popNextMessage<TTLProlongationResponseMessage>();
    if (kMessage->state() == TTLProlongationResponseMessage::Continue) {
        // transactions is still alive and we continue waiting for messages
        debug() << "Transactions is still alive. Continue waiting for messages";
        mStep = Stages::Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes,
             Message::Payments_IntermediateNodeReservationResponse,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(kMaxPathLength));
    }
    return reject("Coordinator send TTL message with transaction finish state. Rolling Back");
}

TransactionResult::SharedConst ReceiverPaymentTransaction::approve()
{
    mCommittedAmount = totalReservedAmount(
        AmountReservation::Incoming);
    BasePaymentTransaction::approve();
    return resultDone();
}

void ReceiverPaymentTransaction::savePaymentOperationIntoHistory(
    IOTransaction::Shared ioTransaction)
{
    debug() << "savePaymentOperationIntoHistory";
    ioTransaction->historyStorage()->savePaymentRecord(
        make_shared<PaymentRecord>(
            currentTransactionUUID(),
            PaymentRecord::PaymentOperationType::IncomingPaymentType,
            // todo : in recovery we don't have this message, need replace on something else
            mMessage->senderUUID,
            mCommittedAmount,
            *mTrustLines->totalBalance().get()),
        mEquivalent);
    debug() << "Operation saved";
}

bool ReceiverPaymentTransaction::checkReservationsDirections() const
{
    debug() << "checkReservationsDirections";
    for (const auto &nodeAndReservations : mReservations) {
        for (const auto &pathIDAndReservation : nodeAndReservations.second) {
            if (pathIDAndReservation.second->direction() != AmountReservation::Incoming) {
                return false;
            }
        }
    }
    return true;
}

const NodeUUID& ReceiverPaymentTransaction::coordinatorUUID() const
{
    return mMessage->senderUUID;
}