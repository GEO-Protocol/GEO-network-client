#include "BasePaymentTransaction.h"

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const SerializedEquivalent equivalent,
    bool iAmGateway,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    ResourcesManager *resourcesManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        type,
        equivalent,
        log),
    mIAmGateway(iAmGateway),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLines),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mResourcesManager(resourcesManager),
    mKeysStore(keystore),
    mSubsystemsController(subsystemsController),
    mTTLRequestWasSend(false),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr),
    mBlockNumberObtainingInProcess(false)
{}

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const SerializedEquivalent equivalent,
    bool iAmGateway,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    ResourcesManager *resourcesManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        type,
        transactionUUID,
        equivalent,
        log),
    mIAmGateway(iAmGateway),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLines),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mResourcesManager(resourcesManager),
    mKeysStore(keystore),
    mSubsystemsController(subsystemsController),
    mTTLRequestWasSend(false),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr),
    mBlockNumberObtainingInProcess(false)
{}

BasePaymentTransaction::BasePaymentTransaction(
    BytesShared buffer,
    bool iAmGateway,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    ResourcesManager *resourcesManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        buffer,
        log),
    mIAmGateway(iAmGateway),
    mTransactionIsVoted(true),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLines),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mResourcesManager(resourcesManager),
    mKeysStore(keystore),
    mSubsystemsController(subsystemsController),
    mCountRecoveryAttempts(0)
{
    auto bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    // mReservations count
    SerializedRecordsCount reservationsCount;
    memcpy(
        &reservationsCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    bool isReserveAmounts = !mTrustLinesManager->isReservationsPresentConsiderTransaction(
        mTransactionUUID);

    // Map values
    for(auto idx = 0; idx < reservationsCount; idx++){
        // Map Key ContractorID
        ContractorID stepContractorID;
        memcpy(
            &stepContractorID,
            buffer.get() + bytesBufferOffset,
            sizeof(ContractorID));
        bytesBufferOffset += sizeof(ContractorID);

        // Map values vector
        SerializedRecordsCount stepReservationVectorSize;
        memcpy(
            &stepReservationVectorSize,
            buffer.get() + bytesBufferOffset,
            sizeof(SerializedRecordsCount));
        bytesBufferOffset += sizeof(SerializedRecordsCount);

        for(auto jdx = 0; jdx < stepReservationVectorSize; jdx++) {

            // PathID
            PathID stepPathID;
            memcpy(
                &stepPathID,
                buffer.get() + bytesBufferOffset,
                sizeof(PathID));
            bytesBufferOffset += sizeof(PathID);

            // Amount
            TrustLineAmount stepAmount;
            vector<byte> amountBytes(
                buffer.get() + bytesBufferOffset,
                buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
            stepAmount = bytesToTrustLineAmount(amountBytes);
            bytesBufferOffset += kTrustLineAmountBytesCount;

            // Direction
            AmountReservation::SerializedReservationDirectionSize stepDirection;
            memcpy(
                &stepDirection,
                buffer.get() + bytesBufferOffset,
                sizeof(AmountReservation::SerializedReservationDirectionSize));
            bytesBufferOffset += sizeof(AmountReservation::SerializedReservationDirectionSize);
            auto stepEnumDirection = static_cast<AmountReservation::ReservationDirection>(stepDirection);

            if (isReserveAmounts) {
                if (stepEnumDirection == AmountReservation::ReservationDirection::Incoming) {
                    if (!reserveIncomingAmount(
                            stepContractorID,
                            stepAmount,
                            stepPathID)) {
                        // can't create reserve, but this reserve was serialized before node dropping
                        // we must stop this node and find out the reason
                        exit(1);
                    }
                }

                if (stepEnumDirection == AmountReservation::ReservationDirection::Outgoing) {
                    if (!reserveOutgoingAmount(
                            stepContractorID,
                            stepAmount,
                            stepPathID)) {
                        // can't create reserve, but this reserve was serialized before node dropping
                        // we must stop this node and find out the reason
                        exit(1);
                    }
                }
            } else {
                if (!copyReservationFromGlobalReservations(
                        stepContractorID,
                        stepAmount,
                        stepEnumDirection,
                        stepPathID)) {
                    // can't get reserve from AmountReseravtionsHandler, but this reserve must be
                    // we must stop this node and find out the reason
                    exit(1);
                }
            }
        }
    }

    // Participants paymentIDs and public keys Part
    SerializedRecordsCount kTotalParticipantsCount;
    memcpy(
        &kTotalParticipantsCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (SerializedRecordNumber idx = 0; idx < kTotalParticipantsCount; idx++) {
        auto *paymentNodeID = new (buffer.get() + bytesBufferOffset) PaymentNodeID;
        bytesBufferOffset += sizeof(PaymentNodeID);
        //---------------------------------------------------
        auto participantContractor = make_shared<Contractor>(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += participantContractor->serializedSize();

        mPaymentParticipants.insert(
            make_pair(
                *paymentNodeID,
                participantContractor));

        auto publicKey = make_shared<lamport::PublicKey>(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += lamport::PublicKey::keySize();

        mParticipantsPublicKeys.insert(
            make_pair(
                *paymentNodeID,
                publicKey));
    }

    memcpy(
        &mMaximalClaimingBlockNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(BlockNumber));

    if (mStep != Stages::Common_Observing) {
        mStep = Stages::Common_Recovery;
        mVotesRecoveryStep = Common_PrepareNodesListToCheckVotes;
    }
}

TransactionResult::SharedConst BasePaymentTransaction::runVotesCheckingStage()
{
    debug() << "runVotesCheckingStage";
    // todo add new stage and remove mTransactionIsVoted
    if (mTransactionIsVoted) {
        return runVotesConsistencyCheckingStage();
    }

    if (! contextIsValid(Message::Payments_ParticipantsPublicKeys)) {
        removeAllDataFromStorageConcerningTransaction();
        return reject("No participants public keys received. Canceling.");
    }

    auto participantsPublicKeyMessage = popNextMessage<ParticipantsPublicKeysMessage>();
    auto coordinator = make_shared<Contractor>(participantsPublicKeyMessage->senderAddresses);
    debug() << "Votes message received from " << coordinator->mainAddress()->fullAddress();
    if (coordinator != mPaymentParticipants[kCoordinatorPaymentNodeID]) {
        warning() << "Wrong coordinator. Continue previous state";
        return resultContinuePreviousState();
    }

    mParticipantsPublicKeys = participantsPublicKeyMessage->publicKeys();

    // todo : check if received own public key is the same as local

    if (!checkPublicKeysAppropriate()) {
        removeAllDataFromStorageConcerningTransaction();
        sendMessage<ParticipantVoteMessage>(
            coordinator->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID());
        return reject("Public keys are not appropriate. Reject.");
    }

    lamport::Signature::Shared signedTransaction;
    try {
        debug() << "Serializing transaction";
        auto ioTransaction = mStorageHandler->beginTransaction();
        auto bytesAndCount = serializeToBytes();
        debug() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        debug() << "Transaction saved";

        auto serializedOwnVotesData = getSerializedParticipantsVotesData(
            make_shared<Contractor>(
                mContractorsManager->ownAddresses()));
        signedTransaction = mKeysStore->signPaymentTransaction(
            ioTransaction,
            currentTransactionUUID(),
            serializedOwnVotesData.first,
            serializedOwnVotesData.second);
        debug() << "Voted +";
        mTransactionIsVoted = true;

        ioTransaction->paymentTransactionsHandler()->saveRecord(
            mTransactionUUID,
            mMaximalClaimingBlockNumber);
    } catch (IOError &e) {
        error() << "Can't sign payment transaction. Details " << e.what();
        removeAllDataFromStorageConcerningTransaction();
        sendMessage<ParticipantVoteMessage>(
            coordinator->mainAddress(),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID());
        return reject("Canceling.");
    }

#ifdef TESTS
    mSubsystemsController->testThrowExceptionOnVoteStage();
    mSubsystemsController->testTerminateProcessOnVoteStage();
    mSubsystemsController->testForbidSendMessageOnVoteConsistencyStage();
    // coordinator wait for this message maxNetworkDelay(6)
    mSubsystemsController->testSleepOnVoteConsistencyStage(
        maxNetworkDelay(8));
#endif

    debug() << "Signed transaction transferred to coordinator";
    sendMessage<ParticipantVoteMessage>(
        coordinator->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID(),
        signedTransaction);

    mStep = Stages::Common_VotesChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxNetworkDelay(6));
}

/*
 * Handles votes list message receiving or it's absence in case,
 * if current node already voted for the operation.
 * In this case transaction can't be simply cancelled,
 * it must be recovered through recover mechanism to keep data integrity.
 */
TransactionResult::SharedConst BasePaymentTransaction::runVotesConsistencyCheckingStage()
{
    debug() << "runVotesConsistencyCheckingStage";

    if (! contextIsValid(Message::Payments_ParticipantsVotes)) {
        // In case if no votes are present - transaction can't be simply cancelled.
        // It must go through recovery stage to avoid inconsistency.
        return recover("No participants votes received.");
    }

#ifdef TESTS
    mSubsystemsController->testThrowExceptionOnVoteConsistencyStage();
    mSubsystemsController->testTerminateProcessOnVoteConsistencyStage();
#endif

    mParticipantsVotesMessage = popNextMessage<ParticipantsVotesMessage>();
    // todo : check if message from coordinator
    debug () << "Participants votes message received.";
    mParticipantsSignatures = mParticipantsVotesMessage->participantsSignatures();

    return processParticipantsVotesMessage();
}

TransactionResult::SharedConst BasePaymentTransaction::processParticipantsVotesMessage()
{
    debug() << "processParticipantsVotesMessage";
    if (!checkSignaturesAppropriate()) {
        return recover("Participants signatures map is incorrect. Rolling back.");
    }
    info() << "All signatures are appropriate";
    auto coordinatorSignature = mParticipantsSignatures[kCoordinatorPaymentNodeID];
    auto coordinatorPublicKey = mParticipantsPublicKeys[kCoordinatorPaymentNodeID];
    auto coordinatorSerializedVotesData = getSerializedParticipantsVotesData(
        mPaymentParticipants[kCoordinatorPaymentNodeID]);
    if (!coordinatorSignature->check(
            coordinatorSerializedVotesData.first.get(),
            coordinatorSerializedVotesData.second,
            coordinatorPublicKey)) {
        recover("Final coordinator signature is wrong");
    }
    for (const auto &paymentNodeIdAndContractor : mPaymentParticipants) {
        if (paymentNodeIdAndContractor.second == mContractorsManager->selfContractor()) {
            // todo discuss if need check own sign
            continue;
        }
        if (paymentNodeIdAndContractor.first == kCoordinatorPaymentNodeID) {
            // coordinator already checked
            continue;
        }
        auto participantPublicKey = mParticipantsPublicKeys[paymentNodeIdAndContractor.first];
        auto participantSignature = mParticipantsSignatures[paymentNodeIdAndContractor.first];
        auto participantSerializedVotesData = getSerializedParticipantsVotesData(
            paymentNodeIdAndContractor.second);
        if (!participantSignature->check(
                participantSerializedVotesData.first.get(),
                participantSerializedVotesData.second,
                participantPublicKey)) {
            warning() << "Node " << paymentNodeIdAndContractor.second->mainAddress()->fullAddress() << " signature is wrong";
            // todo : can be recursive
            return recover("Consensus not achieved.");
        }
    }

    debug() << "Votes list correct. Consensus achieved.";
    return approve();
}

const bool BasePaymentTransaction::reserveOutgoingAmount(
    ContractorID neighborNode,
    const TrustLineAmount& amount,
    const PathID &pathID)
{
    try {
        const auto kReservation = mTrustLinesManager->reserveOutgoingAmount(
            neighborNode,
            currentTransactionUUID(),
            amount);

#ifdef DEBUG
        // todo: uncomment me, when problem with recoverin transaction and reservation after restarting node will be fixed
        //debug() << "Reserved " << amount << " for (" << neighborNode << ") [" << pathID << "] [Outgoing amount reservation].";
#endif

        mReservations[neighborNode].emplace_back(
            pathID,
            kReservation);
        return true;

    } catch (Exception &) {}

    return false;
}

const bool BasePaymentTransaction::reserveIncomingAmount(
    ContractorID neighborNode,
    const TrustLineAmount& amount,
    const PathID &pathID)
{
    try {
        const auto kReservation = mTrustLinesManager->reserveIncomingAmount(
            neighborNode,
            currentTransactionUUID(),
            amount);

#ifdef DEBUG
        // todo: uncomment me, when problem with recoverin transaction and reservation after restarting node will be fixed
        //debug() << "Reserved " << amount << " for (" << neighborNode << ") [" << pathID << "] [Incoming amount reservation].";
#endif

        mReservations[neighborNode].emplace_back(
            pathID,
            kReservation);
        return true;

    } catch (Exception &) {}

    return false;
}

const bool BasePaymentTransaction::copyReservationFromGlobalReservations(
    ContractorID neighborNode,
    const TrustLineAmount &amount,
    AmountReservation::ReservationDirection reservationDirection,
    const PathID &pathID)
{
    try {
        auto reservation = mTrustLinesManager->getAmountReservation(
            neighborNode,
            mTransactionUUID,
            amount,
            reservationDirection);
        mReservations[neighborNode].emplace_back(
            pathID,
            reservation);
        return true;
    } catch (NotFoundError &e) {
        warning() << "copyReservationFromGlobalReservations " << e.what();
        return false;
    }
}

const bool BasePaymentTransaction::contextIsValid(
    Message::MessageType messageType,
    bool showErrorMessage) const
{
    if (mContext.empty()) {
        if (showErrorMessage) {
            warning() << "contextIsValid::context is empty";
        }
        return false;
    }

    if (mContext.size() > 1) {
        if (showErrorMessage) {
            stringstream stream;
            stream << "contextIsValid::context has " << mContext.size() << " messages: ";
            for (auto const &message : mContext) {
                stream << message->typeID() << " ";
            }
            warning() << stream.str();
        }
        return false;
    }

    if (mContext.at(0)->typeID() != messageType) {
        if (showErrorMessage) {
            warning() << "Unexpected message received. (ID " << mContext.at(0)->typeID()
                    << ") It seems that remote node doesn't follows the protocol. Canceling.";
        }

        return false;
    }

    return true;
}

const bool BasePaymentTransaction::resourceIsValid(
    BaseResource::ResourceType resourceType) const
{
    if (mResources.empty()) {
        warning() << "resources are empty";
        return false;
    }
    if (mResources.at(0)->type() != resourceType) {
            warning() << "Unexpected resource received. (ID " << mResources.at(0)->type() << ") Canceling.";
        return false;
    }

    return true;
}

/* Shortcut method for rejecting transaction.
 * Base class realisation contains logic for rejecting (and rolling back)
 * transaction on the nodes where it was launched by the message (and not by the command).
 *
 * Writes error record to the transactions log.
 *
 * WARN:
 * This method must be overloaded on the coordinator side.
 */
TransactionResult::SharedConst BasePaymentTransaction::reject(
    const char *message)
{
    if (message) {
        warning() << message;
    }

    rollBack();
    debug() << "Transaction successfully rolled back.";

    return resultDone();
}

/*
 * Shortcut method for approving transaction.
 * Base class realisation contains logic for approving (and committing)
 * transaction on the nodes where it was launched by the message (and not by the command).
 *
 * Writes success record to the transactions log.
 *
 * WARN:
 * This method must be overloaded on the coordinator side.
 */
TransactionResult::SharedConst BasePaymentTransaction::approve()
{
    debug() << "Transaction approved. Committing.";
    auto ioTransaction = mStorageHandler->beginTransaction();
    commit(ioTransaction);
    savePaymentOperationIntoHistory(ioTransaction);
    return resultDone();
}

void BasePaymentTransaction::commit(
    IOTransaction::Shared ioTransaction)
{
    debug() << "Transaction committing...";

    for (const auto &kNodeIDAndReservations : mReservations) {
        AmountReservation::ReservationDirection reservationDirection;
        for (const auto &kPathIDAndReservation : kNodeIDAndReservations.second) {
            mTrustLinesManager->useReservation(kNodeIDAndReservations.first, kPathIDAndReservation.second);
            if (kPathIDAndReservation.second->direction() == AmountReservation::Outgoing) {
                debug() << "Committed reservation: [ => ] " << kPathIDAndReservation.second->amount()
                        << " for (" << kNodeIDAndReservations.first << ", "
                        << mContractorsManager->contractorMainAddress(kNodeIDAndReservations.first)->fullAddress()
                        << ") [" << kPathIDAndReservation.first << "]";
                mContractorsForCycles.insert(
                    kNodeIDAndReservations.first);
            }
            else if (kPathIDAndReservation.second->direction() == AmountReservation::Incoming) {
                debug() << "Committed reservation: [ <= ] " << kPathIDAndReservation.second->amount()
                        << " for (" << kNodeIDAndReservations.first << ", "
                        << mContractorsManager->contractorMainAddress(kNodeIDAndReservations.first)->fullAddress()
                        << ") [" << kPathIDAndReservation.first << "]";
                if (mIAmGateway) {
                    // gateway try build cycles on both directions, because it don't shared by own routing tables
                    mContractorsForCycles.insert(
                        kNodeIDAndReservations.first);
                }
            }

            reservationDirection = kPathIDAndReservation.second->direction();
            mTrustLinesManager->dropAmountReservation(
                kNodeIDAndReservations.first,
                kPathIDAndReservation.second);
        }
        trustLineActionSignal(
            kNodeIDAndReservations.first,
            mEquivalent,
            reservationDirection == AmountReservation::Outgoing);
    }

    // delete transaction references on dropped reservations
    mReservations.clear();

    // reset initiator cache, because after changing balances
    // we need updated information on max flow calculation transaction
    mTopologyCacheManager->resetInitiatorCache();
    mMaxFlowCacheManager->clearCashes();
    debug() << "Transaction committed.";
    saveVotes(ioTransaction);
    debug() << "Votes saved.";

    // delete this transaction from storage
    ioTransaction->transactionHandler()->deleteRecord(
        currentTransactionUUID());

    // todo : don't send signal if transaction committed after observing
    mTransactionCommittedObservingSignal(
        mTransactionUUID,
        mMaximalClaimingBlockNumber);
}

void BasePaymentTransaction::saveVotes(
    IOTransaction::Shared ioTransaction)
{
    debug() << "saveVotes";
    for (const auto &paymentNodeIdAndContractor : mPaymentParticipants) {
        ioTransaction->paymentParticipantsVotesHandler()->saveRecord(
            mTransactionUUID,
            paymentNodeIdAndContractor.second,
            paymentNodeIdAndContractor.first,
            mParticipantsPublicKeys[paymentNodeIdAndContractor.first],
            mParticipantsSignatures[paymentNodeIdAndContractor.first]);
    }
}

void BasePaymentTransaction::rollBack()
{
    debug() << "rollback";
    // drop reservations in AmountReservationHandler
    for (const auto &kNodeIDAndReservations : mReservations) {
        for (const auto &kPathIDAndReservation : kNodeIDAndReservations.second) {
            mTrustLinesManager->dropAmountReservation(
                kNodeIDAndReservations.first,
                kPathIDAndReservation.second);

            if (kPathIDAndReservation.second->direction() == AmountReservation::Outgoing)
                debug() << "Dropping reservation: [ => ] " << kPathIDAndReservation.second->amount()
                        << " for (" << kNodeIDAndReservations.first << ") [" << kPathIDAndReservation.first << "]";

            else if (kPathIDAndReservation.second->direction() == AmountReservation::Incoming)
                debug() << "Dropping reservation: [ <= ] " << kPathIDAndReservation.second->amount()
                        << " for (" << kNodeIDAndReservations.first << ") [" << kPathIDAndReservation.first << "]";
        }
    }

    // delete transaction references on dropped reservations
    mReservations.clear();
}

void BasePaymentTransaction::rollBack (
    const PathID &pathID)
{
    debug() << "rollback on path";

    auto itNodeIDAndReservations = mReservations.begin();
    while(itNodeIDAndReservations != mReservations.end()) {
        auto itPathIDAndReservation = itNodeIDAndReservations->second.begin();
        while (itPathIDAndReservation != itNodeIDAndReservations->second.end()) {
            if (itPathIDAndReservation->first == pathID) {

                mTrustLinesManager->dropAmountReservation(
                    itNodeIDAndReservations->first,
                    itPathIDAndReservation->second);

                if (itPathIDAndReservation->second->direction() == AmountReservation::Outgoing)
                    debug() << "Dropping reservation: [ => ] " << itPathIDAndReservation->second->amount()
                            << " for (" << itNodeIDAndReservations->first << ") [" << itPathIDAndReservation->first
                            << "]";

                else if (itPathIDAndReservation->second->direction() == AmountReservation::Incoming)
                    debug() << "Dropping reservation: [ <= ] " << itPathIDAndReservation->second->amount()
                            << " for (" << itNodeIDAndReservations->first << ") [" << itPathIDAndReservation->first
                            << "]";

                itPathIDAndReservation = itNodeIDAndReservations->second.erase(itPathIDAndReservation);
                } else {
                    itPathIDAndReservation++;
                }
            }
        if (itNodeIDAndReservations->second.empty()) {
            itNodeIDAndReservations = mReservations.erase(itNodeIDAndReservations);
        } else {
            itNodeIDAndReservations++;
        }
    }
}

void BasePaymentTransaction::removeAllDataFromStorageConcerningTransaction(
    IOTransaction::Shared ioTransaction)
{
    if (ioTransaction == nullptr) {
        ioTransaction = mStorageHandler->beginTransaction();
    }
    ioTransaction->outgoingPaymentReceiptHandler()->deleteRecords(
        mTransactionUUID);
    ioTransaction->incomingPaymentReceiptHandler()->deleteRecords(
        mTransactionUUID);
    ioTransaction->paymentParticipantsVotesHandler()->deleteRecords(
        mTransactionUUID);
    ioTransaction->transactionHandler()->deleteRecord(
        mTransactionUUID);
}

TransactionResult::SharedConst BasePaymentTransaction::recover(
    const char *message)
{
    debug() << "recover";
    if (message != nullptr) {
        warning() << message;
    }

    // todo: expand this condition including observing stage
    if(mTransactionIsVoted){
        mStep = Stages::Common_Recovery;
        mVotesRecoveryStep = VotesRecoveryStages::Common_PrepareNodesListToCheckVotes;
        clearContext();
        mCountRecoveryAttempts = 0;
        return runVotesRecoveryParentStage();
    } else {
        warning() << "Transaction doesn't sent/receive participants votes message and will be closed";
        rollBack();
        return resultDone();
    }
}

uint32_t BasePaymentTransaction::maxNetworkDelay (
    const uint16_t totalHopsCount) const
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(totalHopsCount > 0);
#endif

    return totalHopsCount * kMaxMessageTransferLagMSec;
}

const bool BasePaymentTransaction::shortageReservation (
    ContractorID neighborNode,
    const AmountReservation::ConstShared kReservation,
    const TrustLineAmount &kNewAmount,
    const PathID &pathID)
{
    debug() << "shortageReservation on path " << pathID;
    if (kNewAmount > kReservation->amount()) {
        throw ValueError(
            "BasePaymentTransaction::shortageReservation: "
                "new amount can't be greater than already reserved one.");
    }

    try {
        // this field used only for debug output
        const auto kPreviousAmount = kReservation->amount();

        auto updatedReservation = mTrustLinesManager->updateAmountReservation(
                neighborNode,
                kReservation,
                kNewAmount);

        for (auto it = mReservations[neighborNode].begin(); it != mReservations[neighborNode].end(); it++){
            if ((*it).second.get() == kReservation.get() && (*it).first == pathID) {
                mReservations[neighborNode].erase(it);
                break;
            }
        }
        mReservations[neighborNode].emplace_back(
            pathID,
            updatedReservation);

        if (kReservation->direction() == AmountReservation::Incoming)
            debug() << "Reservation for (" << neighborNode << ") [" << pathID << "] shortened "
                    << "from " << kPreviousAmount << " to " << kNewAmount << " [<=]";
        else
            debug() << "Reservation for (" << neighborNode << ") [" << pathID << "] shortened "
                    << "from " << kPreviousAmount << " to " << kNewAmount << " [=>]";

        return true;

    } catch (NotFoundError &) {}

    return false;
}

void BasePaymentTransaction::dropNodeReservationsOnPath(
    PathID pathID)
{
    debug() << "dropNodeReservationsOnPath: " << pathID;

    auto itNodeReservations = mReservations.begin();
    while (itNodeReservations != mReservations.end()) {
        auto itPathIDAndReservation = itNodeReservations->second.begin();
        while (itPathIDAndReservation != itNodeReservations->second.end()) {
            if (itPathIDAndReservation->first == pathID) {

                mTrustLinesManager->dropAmountReservation(
                        itNodeReservations->first,
                        itPathIDAndReservation->second);

                if (itPathIDAndReservation->second->direction() == AmountReservation::Outgoing)
                    debug() << "Dropping reservation: [ => ] " << itPathIDAndReservation->second->amount()
                            << " for (" << itNodeReservations->first << ") [" << itPathIDAndReservation->first
                            << "]";

                else if (itPathIDAndReservation->second->direction() == AmountReservation::Incoming)
                    debug() << "Dropping reservation: [ <= ] " << itPathIDAndReservation->second->amount()
                            << " for (" << itNodeReservations->first << ") [" << itPathIDAndReservation->first
                            << "]";

                itPathIDAndReservation = itNodeReservations->second.erase(itPathIDAndReservation);
            } else {
                itPathIDAndReservation++;
            }
        }
        if (itNodeReservations->second.empty()) {
            itNodeReservations = mReservations.erase(itNodeReservations);
        } else {
            itNodeReservations++;
        }
    }
}

bool BasePaymentTransaction::updateReservations(
    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmounts)
{
    unordered_set<PathID> updatedPaths;
    const auto reservationsCopy = mReservations;
    for (const auto &nodeAndReservations : reservationsCopy) {
        for (auto pathIDAndReservation : nodeAndReservations.second) {
            const auto updatedPathID = updateReservation(
                nodeAndReservations.first,
                pathIDAndReservation,
                finalAmounts);
            if (updatedPathID != std::numeric_limits<PathID >::max()) {
                updatedPaths.insert(updatedPathID);
            }
        }
    }
    return updatedPaths.size() == finalAmounts.size();
}

PathID BasePaymentTransaction::updateReservation(
    ContractorID contractorID,
    pair<PathID, AmountReservation::ConstShared> &pathIDAndReservation,
    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmounts)
{
    for (auto &pathIDAndAmount : finalAmounts) {
        if (pathIDAndAmount.first == pathIDAndReservation.first) {
            if (*pathIDAndAmount.second.get() != pathIDAndReservation.second->amount()) {
                shortageReservation(
                    contractorID,
                    pathIDAndReservation.second,
                    *pathIDAndAmount.second.get(),
                    pathIDAndAmount.first);
            }
            return pathIDAndAmount.first;
        }
    }
    dropNodeReservationsOnPath(
        pathIDAndReservation.first);
    return std::numeric_limits<PathID >::max();
}

TransactionResult::SharedConst BasePaymentTransaction::runVotesRecoveryParentStage()
{
    debug() << "runVotesRecoveryParentStage";
    switch (mVotesRecoveryStep) {
        case VotesRecoveryStages ::Common_PrepareNodesListToCheckVotes:
            return runPrepareListNodesToCheckNodes();
        case VotesRecoveryStages ::Common_CheckCoordinatorVotesStage:
            return runCheckCoordinatorVotesStage();
        case VotesRecoveryStages ::Common_CheckIntermediateNodeVotesStage:
            return runCheckIntermediateNodeVotesStage();

        default:
            throw RuntimeError(
                "runVotesRecoveryParentStage::run(): "
                    "invalid transaction step.");
    }
}

TransactionResult::SharedConst BasePaymentTransaction::sendVotesRequestMessageAndWaitForResponse(
    Contractor::Shared contractor)
{
    debug() << "sendVotesRequestMessageAndWaitForResponse";
    sendMessage<VotesStatusRequestMessage>(
        contractor->mainAddress(),
        mEquivalent,
        mContractorsManager->ownAddresses(),
        currentTransactionUUID());

    debug() << "Send VotesStatusRequestMessage to " << contractor->mainAddress()->fullAddress();
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst BasePaymentTransaction::runPrepareListNodesToCheckNodes()
{
    debug() << "runPrepareListNodesToCheckNodes";
    // Add all nodes that could be asked for Votes Status.
    // Ignore self and Coordinator Node. Coordinator will be asked first
    Contractor::Shared coordinator = nullptr;
    for(const auto &paymentNodeIdAndContractor: mPaymentParticipants) {
        if (paymentNodeIdAndContractor.second == mContractorsManager->selfContractor()) {
            continue;
        }
        if (paymentNodeIdAndContractor.first == kCoordinatorPaymentNodeID) {
            coordinator = paymentNodeIdAndContractor.second;
            continue;
        }
        mNodesToCheckVotes.push_back(
            paymentNodeIdAndContractor.second);
    }
    mVotesRecoveryStep = VotesRecoveryStages::Common_CheckCoordinatorVotesStage;
    if (coordinator == nullptr) {
        warning() << "Participants list does not contain coordinator";
        return processNextNodeToCheckVotes();
    }
    return sendVotesRequestMessageAndWaitForResponse(coordinator);
}

TransactionResult::SharedConst BasePaymentTransaction::runCheckCoordinatorVotesStage()
{
    debug() << "runCheckCoordinatorVotesStage";
    if (!contextIsValid(Message::Payments_ParticipantsVotes, false)) {
        if (mContext.empty()) {
            debug() << "Coordinator didn't send response";
            return processNextNodeToCheckVotes();
        }
        warning() << "receive message with invalid type, ignore it";
        clearContext();
        return resultContinuePreviousState();
    }

    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    auto coordinator = make_shared<Contractor>(kMessage->senderAddresses);
    if (coordinator != mPaymentParticipants[kCoordinatorPaymentNodeID]) {
        warning() << "Sender " << coordinator->mainAddress()->fullAddress()
                  << " is not coordinator. Ignore this message";
        return resultContinuePreviousState();
    }
    if (kMessage->participantsSignatures().empty()) {
        debug() << "Coordinator don't know result of this transaction yet.";
        return processNextNodeToCheckVotes();
    }

    mParticipantsVotesMessage = kMessage;
    mParticipantsSignatures = mParticipantsVotesMessage->participantsSignatures();

    return processParticipantsVotesMessage();
}

TransactionResult::SharedConst BasePaymentTransaction::runCheckIntermediateNodeVotesStage()
{
    debug() << "runCheckIntermediateNodeVotesStage";
    if (!contextIsValid(Message::Payments_ParticipantsVotes, false)) {
        if (mContext.empty()) {
            debug() << "Intermediate node didn't sent response";
            return processNextNodeToCheckVotes();
        }
        warning() << "receive message with invalid type, ignore it";
        clearContext();
        return resultContinuePreviousState();
    }

    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    auto sender = make_shared<Contractor>(kMessage->senderAddresses);
    info() << "Node " << sender->mainAddress()->fullAddress() << " sent response";

    if (sender != mCurrentNodeToCheckVotes){
        warning() << "Sender is not current checking node";
        return resultContinuePreviousState();
    }

    if (kMessage->participantsSignatures().empty()) {
        debug() << "Intermediate node didn't know about this transaction";
        return processNextNodeToCheckVotes();
    }

    mParticipantsVotesMessage = kMessage;
    mParticipantsSignatures = mParticipantsVotesMessage->participantsSignatures();

    return processParticipantsVotesMessage();
}

TransactionResult::SharedConst BasePaymentTransaction::processNextNodeToCheckVotes()
{
    debug() << "processNextNodeToCheckVotes";
    if (mNodesToCheckVotes.empty()) {
        debug() << "No nodes left to be asked";
        mCountRecoveryAttempts++;
        if (mCountRecoveryAttempts >= kMaxRecoveryAttempts) {
            debug() << "Max count recovery attempts";
            mStep = Stages::Common_Observing;
            return runObservingStage();
        }
        debug() << "Sleep and try again later";
        mVotesRecoveryStep = VotesRecoveryStages::Common_PrepareNodesListToCheckVotes;
        return resultAwakeAfterMilliseconds(
            kWaitMillisecondsToTryRecoverAgain);
    }
    debug() << "Ask another node from payment transaction";
    mVotesRecoveryStep = VotesRecoveryStages::Common_CheckIntermediateNodeVotesStage;
    mCurrentNodeToCheckVotes = mNodesToCheckVotes.back();
    mNodesToCheckVotes.pop_back();
    return sendVotesRequestMessageAndWaitForResponse(
        mCurrentNodeToCheckVotes);
}

const TrustLineAmount BasePaymentTransaction::totalReservedAmount(
    AmountReservation::ReservationDirection reservationDirection) const
{
    TrustLineAmount totalAmount = 0;
    for (const auto &nodeIDAndReservations : mReservations) {
        for (const auto &pathIDAndReservation : nodeIDAndReservations.second) {
            if (pathIDAndReservation.second->direction() == reservationDirection) {
                totalAmount += pathIDAndReservation.second->amount();
            }
        }
    }
    return totalAmount;
}

TransactionResult::SharedConst BasePaymentTransaction::runObservingStage()
{
    info() << "runObservingStage";
    observingClaimSignal(
        make_shared<ObservingClaimAppendRequestMessage>(
            mTransactionUUID,
            mMaximalClaimingBlockNumber,
            mParticipantsPublicKeys));

    debug() << "Serializing transaction";
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto bytesAndCount = serializeToBytes();
    debug() << "Transaction serialized";
    ioTransaction->transactionHandler()->saveRecord(
        currentTransactionUUID(),
        bytesAndCount.first,
        bytesAndCount.second);
    debug() << "Transaction saved";
    // After that control of this transaction will be under ObservingCommunicator
    return resultDone();
}

TransactionResult::SharedConst BasePaymentTransaction::runObservingResultStage()
{
    info() << "runObservingResultStage";
    if (mParticipantsSignatures.empty()) {
        info() << "Close and wait for claiming result";
        observingClaimSignal(
            make_shared<ObservingClaimAppendRequestMessage>(
                mTransactionUUID,
                mMaximalClaimingBlockNumber,
                mParticipantsPublicKeys));
        return resultDone();
    }
    info() << "Participants signatures receive";
    return processParticipantsVotesMessage();
}

TransactionResult::SharedConst BasePaymentTransaction::runObservingRejectTransaction()
{
    info() << "runObservingRejectTransaction";
    auto ioTransaction = mStorageHandler->beginTransaction();
    removeAllDataFromStorageConcerningTransaction(ioTransaction);
    // todo : change transaction history status
    info() << "Reject by expiring claiming time";
    rollBack();
    return resultDone();
}

pair<BytesShared, size_t> BasePaymentTransaction::getSerializedReceipt(
    ContractorID source,
    ContractorID target,
    const TrustLineAmount &amount,
    bool isSource)
{
    size_t serializedDataSize = sizeof(ContractorID)
                                + sizeof(ContractorID)
                                + sizeof(BlockNumber)
                                + TransactionUUID::kBytesSize
                                + kTrustLineAmountBytesCount
                                + sizeof(AuditNumber);
    BytesShared serializedData = tryMalloc(serializedDataSize);

    size_t bytesBufferOffset = 0;
    memcpy(
        serializedData.get() + bytesBufferOffset,
        &source,
        sizeof(ContractorID));
    bytesBufferOffset += sizeof(ContractorID);

    memcpy(
        serializedData.get() + bytesBufferOffset,
        &target,
        sizeof(ContractorID));
    bytesBufferOffset += sizeof(ContractorID);

    memcpy(
        serializedData.get() + bytesBufferOffset,
        &mMaximalClaimingBlockNumber,
        sizeof(BlockNumber));
    bytesBufferOffset += sizeof(BlockNumber);

    memcpy(
        serializedData.get() + bytesBufferOffset,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize);
    bytesBufferOffset += TransactionUUID::kBytesSize;

    auto serializedAmount = trustLineAmountToBytes(amount);
    memcpy(
        serializedData.get() + bytesBufferOffset,
        serializedAmount.data(),
        kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    AuditNumber currentAuditNumber;
    if (isSource) {
        currentAuditNumber = mTrustLinesManager->auditNumber(target);
    } else {
        currentAuditNumber = mTrustLinesManager->auditNumber(source);
    }
    memcpy(
        serializedData.get() + bytesBufferOffset,
        &currentAuditNumber,
        sizeof(AuditNumber));

    debug() << "Receipt " << source << " " << target << " " << amount << " "
            << mMaximalClaimingBlockNumber << " " << currentAuditNumber;

    return make_pair(
        serializedData,
        serializedDataSize);
}

bool BasePaymentTransaction::checkAllNeighborsWithReservationsAreInFinalParticipantsList()
{
    for (const auto &nodeAndReservations : mReservations) {
        auto contractorAddress = mContractorsManager->contractorMainAddress(nodeAndReservations.first);
        if (mPaymentNodesIds.find(contractorAddress->fullAddress()) == mPaymentNodesIds.end()) {
            return false;
        }
    }
    return true;
}

bool BasePaymentTransaction::checkAllPublicKeyHashesProperly()
{
    for (const auto &paymentNodeIdAndContractor : mPaymentParticipants) {
        if (paymentNodeIdAndContractor.first == kCoordinatorPaymentNodeID) {
            continue;
        }
        if (mParticipantsPublicKeysHashes.find(paymentNodeIdAndContractor.second->mainAddress()->fullAddress()) ==
                mParticipantsPublicKeysHashes.end()) {
            warning() << "Public key hash of " << paymentNodeIdAndContractor.second->mainAddress()->fullAddress() << " is absent";
            return false;
        }
        if (mParticipantsPublicKeysHashes[paymentNodeIdAndContractor.second->mainAddress()->fullAddress()].first !=
                paymentNodeIdAndContractor.first) {
            warning() << "Invalid Payment node ID of " << paymentNodeIdAndContractor.second->mainAddress()->fullAddress();
            return false;
        }
    }
    return true;
}

const TrustLineAmount BasePaymentTransaction::totalReservedIncomingAmountToNode(
    ContractorID contractorID)
{
    auto result = TrustLine::kZeroAmount();
    if (mReservations.find(contractorID) == mReservations.end()) {
        return result;
    }
    for (const auto &pathIDAndReservation : mReservations[contractorID]) {
        if (pathIDAndReservation.second->direction() == AmountReservation::Incoming) {
            result += pathIDAndReservation.second->amount();
        }
    }
    return result;
}

bool BasePaymentTransaction::checkPublicKeysAppropriate()
{
    // coordinator don't send own hash, because it send own public key directly
    if (mParticipantsPublicKeys.size() != mParticipantsPublicKeysHashes.size() + 1) {
        warning() << "different numbers of public keys and public keys hashes";
        return false;
    }
    for (const auto &nodeAndPublicKeyHash : mParticipantsPublicKeysHashes) {
        if (mParticipantsPublicKeys.find(nodeAndPublicKeyHash.second.first) == mParticipantsPublicKeys.end()) {
            warning() << "public key from node " << nodeAndPublicKeyHash.first << " ["
                      << nodeAndPublicKeyHash.second.first << "] is absent";
            return false;
        }
        auto publicKey = mParticipantsPublicKeys[nodeAndPublicKeyHash.second.first];
        if (*publicKey->hash() != *nodeAndPublicKeyHash.second.second) {
            warning() << "there are different public key hashes for node " << nodeAndPublicKeyHash.first
                      << " [" << nodeAndPublicKeyHash.second.first << "]";
            return false;
        }
    }
    return true;
}

pair<BytesShared, size_t> BasePaymentTransaction::getSerializedParticipantsVotesData(
    Contractor::Shared contractor)
{
    size_t serializedDataSize = sizeof(BlockNumber)
                                + TransactionUUID::kBytesSize
                                + sizeof(SerializedRecordsCount)
                                + mPaymentParticipants.size() * sizeof(PaymentNodeID);
    BytesShared serializedData = tryMalloc(serializedDataSize);

    size_t bytesBufferOffset = 0;
    memcpy(
        serializedData.get() + bytesBufferOffset,
        &mMaximalClaimingBlockNumber,
        sizeof(BlockNumber));
    bytesBufferOffset += sizeof(BlockNumber);

    memcpy(
        serializedData.get() + bytesBufferOffset,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize);
    bytesBufferOffset += TransactionUUID::kBytesSize;

    auto participantsCount = mPaymentParticipants.size();
    memcpy(
        serializedData.get() + bytesBufferOffset,
        &participantsCount,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (const auto &paymentNodeIdAndContractor : mPaymentParticipants) {
        memcpy(
            serializedData.get() + bytesBufferOffset,
            &paymentNodeIdAndContractor.first,
            sizeof(PaymentNodeID));
        bytesBufferOffset += sizeof(PaymentNodeID);
    }

    return make_pair(
        serializedData,
        serializedDataSize);
}

bool BasePaymentTransaction::checkSignaturesAppropriate()
{
    if (mParticipantsSignatures.size() != mParticipantsPublicKeys.size()) {
        warning() << "different numbers of signatures and participants";
        return false;
    }
    for (const auto &paymentNodeIdAndPubKey : mParticipantsPublicKeys) {
        if (mParticipantsSignatures.find(paymentNodeIdAndPubKey.first) == mParticipantsSignatures.end()) {
            warning() << "there are no signature from participant " << paymentNodeIdAndPubKey.first;
        }
    }
    return true;
}

bool BasePaymentTransaction::checkMaxClaimingBlockNumber(
    BlockNumber maxClaimingBlockNumberOnOwnSide)
{
    if (mMaximalClaimingBlockNumber > maxClaimingBlockNumberOnOwnSide) {
        return mMaximalClaimingBlockNumber - maxClaimingBlockNumberOnOwnSide <= kAllowableBlockNumberDifference;
    }
    if (mMaximalClaimingBlockNumber < maxClaimingBlockNumberOnOwnSide) {
        return maxClaimingBlockNumberOnOwnSide - mMaximalClaimingBlockNumber <= kAllowableBlockNumberDifference;
    }
    return true;
}

pair<BytesShared, size_t> BasePaymentTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedRecordsCount)
                        + reservationsSizeInBytes()
                        + sizeof(SerializedRecordsCount)
                        + sizeof(BlockNumber);
    for (const auto &participant : mPaymentParticipants) {
        bytesCount += sizeof(PaymentNodeID) + participant.second->serializedSize() + lamport::PublicKey::keySize();
    }

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    // Parent part
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    // Reservation Part
    auto kmReservationSize = (SerializedRecordsCount)mReservations.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &kmReservationSize,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for(const auto &nodeAndReservations : mReservations){
        // Map key (ContractorID)
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &nodeAndReservations.first,
            sizeof(ContractorID));
        dataBytesOffset += sizeof(ContractorID);

        // Size of map value vector
        auto kReservationsValueSize = (SerializedRecordsCount)nodeAndReservations.second.size();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &kReservationsValueSize,
            sizeof(SerializedRecordsCount));
        dataBytesOffset += sizeof(SerializedRecordsCount);

        for(const auto &kReservationValues: nodeAndReservations.second){
            // PathID
            memcpy(
                dataBytesShared.get() + dataBytesOffset,
                &kReservationValues.first,
                sizeof(PathID));
            dataBytesOffset += sizeof(PathID);

            // AmountReservation - TrustLineAmount
            vector<byte> buffer = trustLineAmountToBytes(
                kReservationValues.second->amount());
            memcpy(
                dataBytesShared.get() + dataBytesOffset,
                buffer.data(),
                buffer.size());
            dataBytesOffset += buffer.size();

            // Direction
            const auto kDirection = kReservationValues.second->direction();
            memcpy(
                dataBytesShared.get() + dataBytesOffset,
                &kDirection,
                sizeof(AmountReservation::SerializedReservationDirectionSize));
            dataBytesOffset += sizeof(AmountReservation::SerializedReservationDirectionSize);
        }
    }

    // Participants paymentIDs and public keys Part
    auto kTotalParticipantsCount = mPaymentParticipants.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &kTotalParticipantsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    // NodePaymentIDs and payment contractor
    for (auto const &paymentNodeIdAndContractor : mPaymentParticipants) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &paymentNodeIdAndContractor.first,
            sizeof(PaymentNodeID));
        dataBytesOffset += sizeof(PaymentNodeID);

        auto contractorSerializedData = paymentNodeIdAndContractor.second->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            contractorSerializedData.get(),
            paymentNodeIdAndContractor.second->serializedSize());
        dataBytesOffset += paymentNodeIdAndContractor.second->serializedSize();

        auto participantPublicKey = mParticipantsPublicKeys.at(paymentNodeIdAndContractor.first);
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            participantPublicKey->data(),
            lamport::PublicKey::keySize());
        dataBytesOffset += lamport::PublicKey::keySize();
    }

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mMaximalClaimingBlockNumber,
        sizeof(BlockNumber));

    return make_pair(
        dataBytesShared,
        bytesCount);
}

size_t BasePaymentTransaction::reservationsSizeInBytes() const {
    size_t reservationSizeInBytes = 0;
    for (const auto &nodeAndReservations : mReservations){
        reservationSizeInBytes += sizeof(ContractorID)
                                  + nodeAndReservations.second.size() * (
                                    sizeof(PathID) + // PathID
                                    kTrustLineAmountBytesCount +  // Reservation Amount
                                                                  // Reservation Direction
                                    sizeof(AmountReservation::SerializedReservationDirectionSize))
                                  + sizeof(SerializedRecordsCount); // Vector Size

    }
    reservationSizeInBytes += sizeof(SerializedRecordsCount); // map Size
    return reservationSizeInBytes;
}

BaseAddress::Shared BasePaymentTransaction::coordinatorAddress() const
{
    return mContractorsManager->selfContractor()->mainAddress();
}

const SerializedPathLengthSize BasePaymentTransaction::cycleLength() const
{
    return 0;
}

bool BasePaymentTransaction::isVotingStage() const
{
    return mStep == Common_Voting or
            mStep == Common_VotesChecking or
            mStep == Common_Recovery;
}

void BasePaymentTransaction::setTransactionState(
    BasePaymentTransaction::SerializedStep transactionStage)
{
    mStep = transactionStage;
}

void BasePaymentTransaction::setObservingParticipantsSignatures(
    map<PaymentNodeID, lamport::Signature::Shared> participantsSignatures)
{
    mParticipantsSignatures = participantsSignatures;
}

void BasePaymentTransaction::runThreeNodesCyclesTransactions() {
    mBuildCycleThreeNodesSignal(
        mContractorsForCycles,
        mEquivalent);
}

void BasePaymentTransaction::runFourNodesCyclesTransactions() {
    mBuildCycleFourNodesSignal(
        mContractorsForCycles,
        mEquivalent);
}

