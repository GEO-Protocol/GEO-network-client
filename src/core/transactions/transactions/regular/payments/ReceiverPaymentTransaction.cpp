#include "ReceiverPaymentTransaction.h"


ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    ReceiverInitPaymentRequestMessage::ConstShared message,
    bool iAmGateway,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BasePaymentTransaction(
        BaseTransaction::ReceiverPaymentTransaction,
        message->transactionUUID(),
        message->equivalent(),
        iAmGateway,
        contractorsManager,
        trustLines,
        storageHandler,
        topologyCacheManager,
        maxFlowCacheManager,
        keystore,
        log,
        subsystemsController),
    mCoordinator(make_shared<Contractor>(message->senderAddresses)),
    mTransactionAmount(message->amount()),
    mTransactionShouldBeRejected(false)
{
    mStep = Stages::Receiver_CoordinatorRequestApproving;
}

ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    BytesShared buffer,
    bool iAmGateway,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
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
                return runInitializationStage();

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

TransactionResult::SharedConst ReceiverPaymentTransaction::runInitializationStage()
{
    debug() << "Operation for " << mTransactionAmount << " initialised by the:";
    for (const auto &address : mCoordinator->addresses()) {
        debug() << address->fullAddress();
    }

    // Check if total incoming possibilities of the node are <= of the payment amount.
    // If not - there is no reason to process the operation at all.
    // (reject operation)
    const auto kTotalAvailableIncomingAmount = *(mTrustLinesManager->totalIncomingAmount());
    debug() << "Total incoming amount: " << kTotalAvailableIncomingAmount;
    if (kTotalAvailableIncomingAmount < mTransactionAmount) {
        sendMessage<ReceiverInitPaymentResponseMessage>(
            mCoordinator->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            ReceiverInitPaymentResponseMessage::Rejected);

        info() << "Operation rejected due to insufficient funds.";
        return resultDone();
    }

    sendMessage<ReceiverInitPaymentResponseMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        ReceiverInitPaymentResponseMessage::Accepted);

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
        debug() << "Send TTLTransaction message to coordinator " << mCoordinator->mainAddress();
        sendMessage<TTLProlongationRequestMessage>(
            mCoordinator->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID());
        mStep = Stages::Common_ClarificationTransactionBeforeVoting;
        return resultWaitForMessageTypes(
            {Message::Payments_TTLProlongationResponse,
             Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay(2));
    }

    const auto kMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();

    const auto kNeighbor = kMessage->senderAddresses.at(0);
    if (kMessage->finalAmountsConfiguration().empty()) {
        warning() << "Reservation vector is empty";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            0,                  // 0, because we don't know pathID
            ResponseMessage::Closed);
    }

    const auto kReservation = kMessage->finalAmountsConfiguration()[0];
    debug() << "Amount reservation for " << *kReservation.second.get() << " request received from "
            << kNeighbor->fullAddress() << " [" << kReservation.first << "]";
    debug() << "Received reservations size: " << kMessage->finalAmountsConfiguration().size();

    auto neighborID = mContractorsManager->contractorIDByAddress(kNeighbor);
    if (neighborID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Previous node is not a neighbor";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Rejected);
    }

    if (! mTrustLinesManager->trustLineIsPresent(neighborID)) {
        warning() << "Path is not valid: there is no TL with previous node. Rejected.";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Rejected);
    }

    if (! mTrustLinesManager->trustLineIsActive(neighborID)) {
        warning() << "Path is not valid: TL with previous node is not active. Rejected.";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Rejected);
    }

    // TODO: enhance this check
    // Neighbor public key must be used here.

    // update local reservations during amounts from coordinator
    if (!updateReservations(vector<pair<PathID, ConstSharedTrustLineAmount>>(
            kMessage->finalAmountsConfiguration().begin() + 1,
            kMessage->finalAmountsConfiguration().end()))) {
        warning() << "Previous node send path configuration, which is absent on current node";
        // next loop is only logger info
        for (const auto &reservation : kMessage->finalAmountsConfiguration()) {
            debug() << "path: " << reservation.first << " amount: " << *reservation.second.get();
        }
        mTransactionShouldBeRejected = true;
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Closed);
    }
    debug() << "All reservations were updated";

    if (!mTrustLinesManager->trustLineContractorKeysPresent(neighborID)) {
        warning() << "There are no contractor keys on TL";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::RejectedDueContractorKeysAbsence);
    }

    // Note: copy of shared pointer is required.
    const auto kAvailableAmount = mTrustLinesManager->incomingTrustAmountConsideringReservations(neighborID);
    if (*kAvailableAmount == TrustLine::kZeroAmount()) {
        warning() << "Available amount equals zero. Reservation reject.";
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Rejected);
    }

    debug() << "Available amount " << *kAvailableAmount;
    const auto kReservationAmount = min(
        *kReservation.second.get(),
        *kAvailableAmount);

#ifdef TESTS
    // todo : adapt SubsystemsController
//    if (kMessage->senderUUID == mMessage->senderUUID) {
//        mSubsystemsController->testForbidSendMessageToCoordinatorOnReservationStage(
//            NodeUUID::empty(),
//            TrustLine::kZeroAmount());
//    }
//    mSubsystemsController->testForbidSendResponseToIntNodeOnReservationStage(
//        kMessage->senderUUID,
//        kReservationAmount);
    mSubsystemsController->testThrowExceptionOnPreviousNeighborRequestProcessingStage();
    mSubsystemsController->testTerminateProcessOnPreviousNeighborRequestProcessingStage();
#endif

    if (! reserveIncomingAmount(neighborID, kReservationAmount, kReservation.first)) {
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
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Rejected);
    }

    const auto kTotalReserved = totalReservedAmount(
            AmountReservation::Incoming);
    if (kTotalReserved > mTransactionAmount){
        warning() << "Reserved amount is greater than requested. It indicates protocol or realisation error.";
        mTransactionShouldBeRejected = true;
        return sendErrorMessageOnPreviousNodeRequest(
            kNeighbor,
            kReservation.first,
            ResponseMessage::Closed);
    }

    debug() << "Reserved locally: " << kReservationAmount;
    sendMessage<IntermediateNodeReservationResponseMessage>(
        kNeighbor,
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        kReservation.first,
        ResponseMessage::Accepted,
        kReservationAmount);

    if (kTotalReserved == mTransactionAmount) {
        // Reserved amount is enough to move to votes processing stage.
        info() << "Requested amount collected.";

        // TODO: receiver must know, how many paths are involved into the transaction.
        // This info helps to calculate max timeout,
        // that would be used for waiting for votes list message.

        mStep = Stages::Coordinator_FinalAmountsConfigurationConfirmation;
        return resultWaitForMessageTypes(
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_TransactionPublicKeyHash,
             Message::Payments_IntermediateNodeReservationRequest,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(kMaxPathLength - 1));

    }

    // Waiting for another reservation request
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay((kMaxPathLength - 1) * 4));
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

    if (contextIsValid(Message::Payments_TransactionPublicKeyHash, false)) {
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
                 Message::Payments_TransactionPublicKeyHash},
                maxNetworkDelay((kMaxPathLength - 1) * 4));
        }
        return reject("Coordinator send TTL message with transaction finish state. Rolling Back");
    }

    debug() << "Send TTLTransaction message to coordinator " << mCoordinator->mainAddress();
    sendMessage<TTLProlongationRequestMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID());
    mStep = Common_ClarificationTransactionDuringFinalAmountsClarification;

    return resultWaitForMessageTypes(
        {Message::Payments_FinalAmountsConfiguration,
         Message::Payments_TransactionPublicKeyHash,
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
    auto coordinatorAddress = kMessage->senderAddresses.at(0);
    // todo : check coordinator
    if (!updateReservations(
            kMessage->finalAmountsConfiguration())) {
        sendErrorMessageOnFinalAmountsConfiguration();
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

    mPaymentParticipants = kMessage->paymentParticipants();
    // todo check if current node is present in mPaymentParticipants
    for (const auto &paymentNodeIdAndAddress : mPaymentParticipants) {
        mPaymentNodesIds.insert(
            make_pair(
                paymentNodeIdAndAddress.second->mainAddress()->fullAddress(),
                paymentNodeIdAndAddress.first));
    }
    if (!checkAllNeighborsWithReservationsAreInFinalParticipantsList()) {
        sendErrorMessageOnFinalAmountsConfiguration();
        return reject("Node has reservation with participant, which not included in mPaymentNodesIds. Rejected");
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (kMessage->isReceiptContains()) {
        info() << "Coordinator also send receipt";
        auto coordinatorID = mContractorsManager->contractorIDByAddress(kMessage->senderAddresses.at(0));
        if (coordinatorID == ContractorsManager::kNotFoundContractorID) {
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
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Can't save coordinator receipt. Rejected.");
        }
        info() << "Coordinator's receipt is valid";
    } else {
        auto coordinatorID = mContractorsManager->contractorIDByAddress(kMessage->senderAddresses.at(0));
        if (coordinatorID != ContractorsManager::kNotFoundContractorID) {
            auto coordinatorTotalIncomingReservationAmount = totalReservedIncomingAmountToNode(
                coordinatorID);
            if (coordinatorTotalIncomingReservationAmount != TrustLine::kZeroAmount()) {
                sendErrorMessageOnFinalAmountsConfiguration();
                warning() << "Receipt amount: 0. Local incoming amount: " << coordinatorTotalIncomingReservationAmount;
                return reject("Coordinator send invalid receipt amount. Rejected");
            }
        }
    }

    mPublicKey = mKeysStore->generateAndSaveKeyPairForPaymentTransaction(
        ioTransaction,
        currentTransactionUUID());
    mParticipantsPublicKeysHashes.insert(
        make_pair(
            mContractorsManager->ownAddresses().at(0)->fullAddress(),
            make_pair(
                mPaymentNodesIds[mContractorsManager->ownAddresses().at(0)->fullAddress()],
                mPublicKey->hash())));

    // send public key hash to all participants except coordinator
    auto ownPaymentID = mPaymentNodesIds[mContractorsManager->ownAddresses().at(0)->fullAddress()];
    for (const auto &paymentNodeIdAndContractor : mPaymentParticipants) {
        if (paymentNodeIdAndContractor.second == mCoordinator) {
            continue;
        }
        if (paymentNodeIdAndContractor.second == mContractorsManager->selfContractor()) {
            continue;
        }
        info() << "Send public key hash to " << paymentNodeIdAndContractor.second->mainAddress()->fullAddress();
        sendMessage<TransactionPublicKeyHashMessage>(
            paymentNodeIdAndContractor.second->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            ownPaymentID,
            mPublicKey->hash());
    }

    // coordinator don't send public key hash
    if (mPaymentParticipants.size() == mParticipantsPublicKeysHashes.size() + 1) {
        info() << "All neighbors send theirs reservations";
        // all neighbors sent theirs reservations
        if (!checkAllPublicKeyHashesProperly()) {
            sendErrorMessageOnFinalAmountsConfiguration();
            return reject("Public key hashes are not properly. Rejected");
        }
        info() << "All public key hashes are properly";

        sendMessage<FinalAmountsConfigurationResponseMessage>(
            mCoordinator->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
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

TransactionResult::SharedConst ReceiverPaymentTransaction::runFinalReservationsNeighborConfirmation()
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
            {Message::Payments_FinalAmountsConfiguration,
             Message::Payments_TransactionPublicKeyHash,
             Message::Payments_TTLProlongationResponse},
            maxNetworkDelay(1));
    }

    // coordinator already sent final amounts configuration
    // coordinator don't send public key hash
    if (mPaymentParticipants.size() == mParticipantsPublicKeysHashes.size() + 1) {
        // all neighbors sent theirs reservations
        if (!checkAllPublicKeyHashesProperly()) {
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

TransactionResult::SharedConst ReceiverPaymentTransaction::runClarificationOfTransactionDuringFinalAmountsClarification()
{
    debug() << "runClarificationOfTransactionDuringFinalAmountsClarification";
    if (contextIsValid(Message::Payments_IntermediateNodeReservationRequest, false)) {
        return runAmountReservationStage();
    }

    if (contextIsValid(Message::Payments_FinalAmountsConfiguration, false) or
            contextIsValid(Message::Payments_TransactionPublicKeyHash, false)) {
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

    if (contextIsValid(Message::Payments_ParticipantsPublicKeys, false) or
            contextIsValid(Message::Payments_ParticipantsVotes, false)) {
        if (mTransactionShouldBeRejected) {
            // this case can happens only with Receiver,
            // when coordinator wants to reserve greater then command amount
            reject("Receiver rejected transaction because of discrepancy reservations with Coordinator. Rolling back.");
        }
        return runVotesCheckingStage();
    }

    debug() << "Send TTLTransaction message to coordinator " << mCoordinator->mainAddress()->fullAddress();
    sendMessage<TTLProlongationRequestMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID());
    mStep = Stages::Common_ClarificationTransactionDuringVoting;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsPublicKeys,
         Message::Payments_ParticipantsVotes,
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

    if (contextIsValid(Message::MessageType::Payments_ParticipantsPublicKeys, false) or
            contextIsValid(Message::Payments_ParticipantsVotes, false)) {
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
            {Message::Payments_ParticipantsPublicKeys,
             Message::Payments_ParticipantsVotes,
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
    if (mPaymentParticipants.count(kCoordinatorPaymentNodeID) == 0) {
        warning() << "Can't identify coordinator node";
        // todo : need correct reaction
    }
    auto coordinatorAddress = mPaymentParticipants[kCoordinatorPaymentNodeID];
    ioTransaction->historyStorage()->savePaymentRecord(
        make_shared<PaymentRecord>(
            currentTransactionUUID(),
            PaymentRecord::PaymentOperationType::IncomingPaymentType,
            mCoordinator,
            mCommittedAmount,
            *mTrustLinesManager->totalBalance().get()),
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

TransactionResult::SharedConst ReceiverPaymentTransaction::sendErrorMessageOnPreviousNodeRequest(
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

    // TODO: enhancement: send approximate paths count to receiver, so it will be able to wait correct timeout.
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationRequest,
         Message::Payments_TTLProlongationResponse},
        maxNetworkDelay((kMaxPathLength - 1) * 4));
}

void ReceiverPaymentTransaction::sendErrorMessageOnFinalAmountsConfiguration()
{
    sendMessage<FinalAmountsConfigurationResponseMessage>(
        mCoordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        FinalAmountsConfigurationResponseMessage::Rejected);
}

BaseAddress::Shared ReceiverPaymentTransaction::coordinatorAddress() const
{
    return mCoordinator->mainAddress();
}

const string ReceiverPaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiverPaymentTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}