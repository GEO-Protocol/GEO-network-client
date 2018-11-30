#include "BasePaymentTransaction.h"

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const NodeUUID &currentNodeUUID,
    const SerializedEquivalent equivalent,
    bool iAmGateway,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        type,
        currentNodeUUID,
        equivalent,
        log),
    mIAmGateway(iAmGateway),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mKeysStore(keystore),
    mSubsystemsController(subsystemsController),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr)
{}

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &currentNodeUUID,
    const SerializedEquivalent equivalent,
    bool iAmGateway,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        type,
        transactionUUID,
        currentNodeUUID,
        equivalent,
        log),
    mIAmGateway(iAmGateway),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mKeysStore(keystore),
    mSubsystemsController(subsystemsController),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr)
{}

BasePaymentTransaction::BasePaymentTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    bool iAmGateway,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore *keystore,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        buffer,
        nodeUUID,
        log),
    mIAmGateway(iAmGateway),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mKeysStore(keystore),
    mSubsystemsController(subsystemsController)
{
    auto bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    mStep = Stages::Common_Recovery;

    // mReservations count
    SerializedRecordsCount reservationsCount;
    memcpy(
        &reservationsCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    // Map values
    for(auto i=1; i<=reservationsCount; i++){
        // Map Key NodeUUID
        NodeUUID stepNodeUUID(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;

        // Map values vector
        SerializedRecordsCount stepReservationVectorSize;
        memcpy(
            &stepReservationVectorSize,
            buffer.get() + bytesBufferOffset,
            sizeof(SerializedRecordsCount));
        bytesBufferOffset += sizeof(SerializedRecordsCount);

        vector<pair<PathID, AmountReservation::ConstShared>> stepVector;
        for(auto j=1; j<=stepReservationVectorSize; j++) {

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

            // Transaction UUID
            TransactionUUID stepTransactionUUID(buffer.get() + bytesBufferOffset);
            bytesBufferOffset += TransactionUUID::kBytesSize;

            // Direction
            AmountReservation::SerializedReservationDirectionSize stepDirection;
            memcpy(
                &stepDirection,
                buffer.get() + bytesBufferOffset,
                sizeof(AmountReservation::SerializedReservationDirectionSize));
            bytesBufferOffset += sizeof(AmountReservation::SerializedReservationDirectionSize);
            auto stepEnumDirection = static_cast<AmountReservation::ReservationDirection>(stepDirection);

            auto stepAmountReservation = make_shared<AmountReservation>(
                stepTransactionUUID,
                stepAmount,
                stepEnumDirection);
            stepVector.emplace_back(
                stepPathID,
                stepAmountReservation);

            if (stepDirection == AmountReservation::ReservationDirection::Incoming) {
                if (!reserveIncomingAmount(
                    stepNodeUUID,
                    stepAmount,
                    stepPathID)) {
                    // can't create reserve, but this reserve was serialized before node dropping
                    // we must stop this node and find out the reason
                    exit(1);
                }
            }

            if (stepDirection == AmountReservation::ReservationDirection::Outgoing) {
                if (!reserveOutgoingAmount(
                    stepNodeUUID,
                    stepAmount,
                    stepPathID)) {
                    // can't create reserve, but this reserve was serialized before node dropping
                    // we must stop this node and find out the reason
                    exit(1);
                }
            }
        }
        mReservations.insert(
            make_pair(
                stepNodeUUID,
                stepVector));
    }

    // Participants paymentIDs and public keys Part
    SerializedRecordsCount kTotalParticipantsCount;
    memcpy(
        &kTotalParticipantsCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (SerializedRecordNumber idx = 0; idx < kTotalParticipantsCount; idx++) {
        NodeUUID stepNodeUUID(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        auto *paymentNodeID = new (buffer.get() + bytesBufferOffset) PaymentNodeID;
        bytesBufferOffset += sizeof(PaymentNodeID);
        //---------------------------------------------------
        mPaymentNodesIds.insert(
            make_pair(
                stepNodeUUID,
                *paymentNodeID));
    }

    for (SerializedRecordNumber i = 0; i < kTotalParticipantsCount; ++i) {
        PaymentNodeID paymentNodeID;
        memcpy(
            &paymentNodeID,
            buffer.get() + bytesBufferOffset,
            sizeof(PaymentNodeID));
        bytesBufferOffset += sizeof(PaymentNodeID);

        auto publicKey = make_shared<lamport::PublicKey>(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += lamport::PublicKey::keySize();

        mParticipantsPublicKeys.insert(
            make_pair(
                paymentNodeID,
                publicKey));
    }
}

TransactionResult::SharedConst BasePaymentTransaction::runVotesCheckingStage()
{
    debug() << "runVotesCheckingStage";
    // todo add new stage and remove mTransactionIsVoted
    if (mTransactionIsVoted)
        return runVotesConsistencyCheckingStage();

    if (! contextIsValid(Message::Payments_ParticipantsPublicKeys))
        return reject("No participants public keys received. Canceling.");

    auto participantsPublicKeyMessage = popNextMessage<ParticipantsPublicKeysMessage>();
    mParticipantsPublicKeys = participantsPublicKeyMessage->publicKeys();

    debug() << "Votes message received";

    // todo : check if received own public key is the same as local

    if (!checkPublicKeysAppropriate()) {
        return reject("Public keys are not appropriate. Reject.");
    }

    try {
        // Check if current node is listed in the votes list.
        // This check is needed to prevent processing message in case of missdelivering.
        // todo : discuss if node sign Reject

    } catch (NotFoundError &) {
        // todo appropriate reaction
        // It seems that current node wasn't listed in the votes list.
        // This is possible only in case, when one node takes part in 2 parallel transactions,
        // that have common UUID (transactions UUIDs collision).
        // The probability of this is very small, but is present.
        //
        // In this case - the message must be simply ignored.

        warning() << "Votes message ignored due to transactions UUIDs collision detected.";
        debug() << "Waiting for another votes message.";

        return resultContinuePreviousState();
    }

#ifdef TESTS
    mSubsystemsController->testThrowExceptionOnVoteStage();
    mSubsystemsController->testTerminateProcessOnVoteStage();
#endif

    {
        debug() << "Serializing transaction";
        auto ioTransaction = mStorageHandler->beginTransaction();
        auto bytesAndCount = serializeToBytes();
        debug() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        debug() << "Transaction saved";
    }

    auto serializedOwnVotesData = getSerializedParticipantsVotesData(mNodeUUID);
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto signedTransaction = mKeysStore->signPaymentTransaction(
        ioTransaction,
        currentTransactionUUID(),
        serializedOwnVotesData.first,
        serializedOwnVotesData.second);
    debug() << "Voted +";
    mTransactionIsVoted = true;

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToCoordinatorOnVoteStage();
    mSubsystemsController->testSleepOnVoteConsistencyStage(
        maxNetworkDelay(
            mPaymentNodesIds.size() + 2));
#endif

    debug() << "Signed transaction transferred to coordinator " << participantsPublicKeyMessage->senderUUID;
    sendMessage<ParticipantVoteMessage>(
        participantsPublicKeyMessage->senderUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        signedTransaction);

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
    mSubsystemsController->testForbidSendMessageOnVoteConsistencyStage();
    mSubsystemsController->testThrowExceptionOnVoteConsistencyStage();
    mSubsystemsController->testTerminateProcessOnVoteConsistencyStage();
#endif

    mParticipantsVotesMessage = popNextMessage<ParticipantsVotesMessage>();
    debug () << "Participants votes message received.";
    mParticipantsSignatures = mParticipantsVotesMessage->participantsSignatures();

    return processParticipantsVotesMessage();
}

TransactionResult::SharedConst BasePaymentTransaction::processParticipantsVotesMessage()
{
    debug() << "processParticipantsVotesMessage";
    if (!checkSignsAppropriate()) {
        return reject("Participants signs map is incorrect. Rolling back.");
    }
    info() << "All signatures are appropriate";

    PaymentNodeID coordinatorID = kCoordinatorPaymentNodeID;
    auto coordinatorSign = mParticipantsSignatures[coordinatorID];
    auto coordinatorPublicKey = mParticipantsPublicKeys[coordinatorID];
    auto coordinatorSerializedVotesData = getSerializedParticipantsVotesData(
        mParticipantsVotesMessage->senderUUID);
    if (!coordinatorSign->check(
            coordinatorSerializedVotesData.first.get(),
            coordinatorSerializedVotesData.second,
            coordinatorPublicKey)) {
        reject("Final coordinator signature is wrong");
    }
    for (const auto &nodeUUIDAndPaymentNodeID : mPaymentNodesIds) {
        if (nodeUUIDAndPaymentNodeID.first == mNodeUUID) {
            // todo discuss if need check own sign
            continue;
        }
        if (nodeUUIDAndPaymentNodeID.first == mParticipantsVotesMessage->senderUUID) {
            // coordinator already checked
            continue;
        }
        auto participantPublicKey = mParticipantsPublicKeys[nodeUUIDAndPaymentNodeID.second];
        auto participantSign = mParticipantsSignatures[nodeUUIDAndPaymentNodeID.second];
        auto participantSerializedVotesData = getSerializedParticipantsVotesData(
                nodeUUIDAndPaymentNodeID.first);
        if (!participantSign->check(
                participantSerializedVotesData.first.get(),
                participantSerializedVotesData.second,
                participantPublicKey)) {
            warning() << "Final node " << nodeUUIDAndPaymentNodeID.first.stringUUID() << " sign is wrong";
            return reject("Consensus not achieved.");
        }
    }

    debug() << "Votes list correct. Consensus achieved.";
    return approve();
}

const bool BasePaymentTransaction::reserveOutgoingAmount(
    const NodeUUID& neighborNode,
    const TrustLineAmount& amount,
    const PathID &pathID)
{
    try {
        const auto kReservation = mTrustLines->reserveOutgoingAmount(
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
    const NodeUUID& neighborNode,
    const TrustLineAmount& amount,
    const PathID &pathID)
{
    try {
        const auto kReservation = mTrustLines->reserveIncomingAmount(
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

    for (const auto &kNodeUUIDAndReservations : mReservations) {
        AmountReservation::ReservationDirection reservationDirection;
        for (const auto &kPathIDAndReservation : kNodeUUIDAndReservations.second) {
            mTrustLines->useReservation(kNodeUUIDAndReservations.first, kPathIDAndReservation.second);
            if (kPathIDAndReservation.second->direction() == AmountReservation::Outgoing) {
                debug() << "Committed reservation: [ => ] " << kPathIDAndReservation.second->amount()
                        << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathIDAndReservation.first
                        << "]";
                mCreditorsForCycles.insert(kNodeUUIDAndReservations.first);
            }
            else if (kPathIDAndReservation.second->direction() == AmountReservation::Incoming) {
                debug() << "Committed reservation: [ <= ] " << kPathIDAndReservation.second->amount()
                        << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathIDAndReservation.first
                        << "]";
                if (mIAmGateway) {
                    // gateway try build cycles on both directions, because it don't shared by own routing tables
                    mCreditorsForCycles.insert(kNodeUUIDAndReservations.first);
                }
            }

            reservationDirection = kPathIDAndReservation.second->direction();
            mTrustLines->dropAmountReservation(
                kNodeUUIDAndReservations.first,
                kPathIDAndReservation.second);
        }
        trustLineActionSignal(
            mTrustLines->contractorID(kNodeUUIDAndReservations.first),
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
}

void BasePaymentTransaction::saveVotes(
    IOTransaction::Shared ioTransaction)
{
    debug() << "saveVotes";
    for (const auto &nodeUUIDAndPaymentNodeID : mPaymentNodesIds) {
        ioTransaction->paymentParticipantsVotesHandler()->saveRecord(
            mTransactionUUID,
            nodeUUIDAndPaymentNodeID.first,
            nodeUUIDAndPaymentNodeID.second,
            mParticipantsPublicKeys[nodeUUIDAndPaymentNodeID.second],
            mParticipantsSignatures[nodeUUIDAndPaymentNodeID.second]);
    }
}

void BasePaymentTransaction::rollBack ()
{
    debug() << "rollback";
    const auto ioTransaction = mStorageHandler->beginTransaction();

    // drop reservations in AmountReservationHandler
    for (const auto &kNodeUUIDAndReservations : mReservations) {
        for (const auto &kPathIDAndReservation : kNodeUUIDAndReservations.second) {
            mTrustLines->dropAmountReservation(
                kNodeUUIDAndReservations.first,
                kPathIDAndReservation.second);

            if (kPathIDAndReservation.second->direction() == AmountReservation::Outgoing)
                debug() << "Dropping reservation: [ => ] " << kPathIDAndReservation.second->amount()
                        << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathIDAndReservation.first << "]";

            else if (kPathIDAndReservation.second->direction() == AmountReservation::Incoming)
                debug() << "Dropping reservation: [ <= ] " << kPathIDAndReservation.second->amount()
                        << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathIDAndReservation.first << "]";
        }
    }

    // delete transaction references on dropped reservations
    mReservations.clear();

    ioTransaction->transactionHandler()->deleteRecord(currentTransactionUUID());
}

void BasePaymentTransaction::rollBack (
    const PathID &pathID)
{
    debug() << "rollback on path";

    auto itNodeUUIDAndReservations = mReservations.begin();
    while(itNodeUUIDAndReservations != mReservations.end()) {
        auto itPathIDAndReservation = itNodeUUIDAndReservations->second.begin();
        while (itPathIDAndReservation != itNodeUUIDAndReservations->second.end()) {
            if (itPathIDAndReservation->first == pathID) {

                mTrustLines->dropAmountReservation(
                    itNodeUUIDAndReservations->first,
                    itPathIDAndReservation->second);

                if (itPathIDAndReservation->second->direction() == AmountReservation::Outgoing)
                    debug() << "Dropping reservation: [ => ] " << itPathIDAndReservation->second->amount()
                           << " for (" << itNodeUUIDAndReservations->first << ") [" << itPathIDAndReservation->first
                           << "]";

                else if (itPathIDAndReservation->second->direction() == AmountReservation::Incoming)
                    debug() << "Dropping reservation: [ <= ] " << itPathIDAndReservation->second->amount()
                           << " for (" << itNodeUUIDAndReservations->first << ") [" << itPathIDAndReservation->first
                           << "]";

                itPathIDAndReservation = itNodeUUIDAndReservations->second.erase(itPathIDAndReservation);
                } else {
                    itPathIDAndReservation++;
                }
            }
        if (itNodeUUIDAndReservations->second.empty()) {
            itNodeUUIDAndReservations = mReservations.erase(itNodeUUIDAndReservations);
        } else {
            itNodeUUIDAndReservations++;
        }
    }
}

TransactionResult::SharedConst BasePaymentTransaction::recover(
    const char *message)
{
    debug() << "recover";
    if (message != nullptr) {
        warning() << message;
    }

    if(mTransactionIsVoted){
        mStep = Stages::Common_Recovery;
        mVotesRecoveryStep = VotesRecoveryStages::Common_PrepareNodesListToCheckVotes;
        clearContext();
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
    const NodeUUID kContractor,
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

        auto updatedReservation = mTrustLines->updateAmountReservation(
            kContractor,
            kReservation,
            kNewAmount);

        for (auto it = mReservations[kContractor].begin(); it != mReservations[kContractor].end(); it++){
            if ((*it).second.get() == kReservation.get() && (*it).first == pathID) {
                mReservations[kContractor].erase(it);
                break;
            }
        }
        mReservations[kContractor].emplace_back(
            pathID,
            updatedReservation);

        if (kReservation->direction() == AmountReservation::Incoming)
            debug() << "Reservation for (" << kContractor << ") [" << pathID << "] shortened "
                   << "from " << kPreviousAmount << " to " << kNewAmount << " [<=]";
        else
            debug() << "Reservation for (" << kContractor << ") [" << pathID << "] shortened "
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

                mTrustLines->dropAmountReservation(
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
    const NodeUUID &contractorUUID,
    pair<PathID, AmountReservation::ConstShared> &pathIDAndReservation,
    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmounts)
{
    for (auto &pathIDAndAmount : finalAmounts) {
        if (pathIDAndAmount.first == pathIDAndReservation.first) {
            if (*pathIDAndAmount.second.get() != pathIDAndReservation.second->amount()) {
                shortageReservation(
                    contractorUUID,
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
    const NodeUUID &contractorUUID)
{
    debug() << "sendVotesRequestMessageAndWaitForResponse";
    auto requestMessage = make_shared<VotesStatusRequestMessage>(
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID());

    sendMessage(
        contractorUUID,
        requestMessage);

    debug() << "Send VotesStatusRequestMessage to " << contractorUUID;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst BasePaymentTransaction::runPrepareListNodesToCheckNodes()
{
    debug() << "runPrepareListNodesToCheckNodes";
    // Add all nodes that could be asked for Votes Status.
    // Ignore self and Coordinator Node. Coordinator will be asked first
    auto kCoordinatorUUID = NodeUUID::empty();
    for(const auto &kNodeUUIDAndPaymentNodeID: mPaymentNodesIds) {
        if (kNodeUUIDAndPaymentNodeID.first == mNodeUUID) {
            continue;
        }
        if (kNodeUUIDAndPaymentNodeID.second == kCoordinatorPaymentNodeID) {
            kCoordinatorUUID = kNodeUUIDAndPaymentNodeID.first;
            continue;
        }
        mNodesToCheckVotes.push_back(
            kNodeUUIDAndPaymentNodeID.first);
    }
    mVotesRecoveryStep = VotesRecoveryStages::Common_CheckCoordinatorVotesStage;
    return sendVotesRequestMessageAndWaitForResponse(kCoordinatorUUID);
}

TransactionResult::SharedConst BasePaymentTransaction::runCheckCoordinatorVotesStage()
{
    debug() << "runCheckCoordinatorVotesStage";
    if (!contextIsValid(Message::Payments_ParticipantsVotes, false)) {
        if (mContext.empty()) {
            debug() << "Coordinator didn't sent response";
            return processNextNodeToCheckVotes();
        }
        warning() << "receive message with invalid type, ignore it";
        clearContext();
        return resultContinuePreviousState();
    }

    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    if (kMessage->participantsSignatures().empty()) {
        debug() << "Coordinator don't know result of this transaction yet. Sleep.";
        mVotesRecoveryStep = VotesRecoveryStages::Common_PrepareNodesListToCheckVotes;
        return resultAwakeAfterMilliseconds(
            kWaitMillisecondsToTryRecoverAgain);
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
    const auto kSenderUUID = kMessage->senderUUID;

    if (kMessage->participantsSignatures().empty()) {
        debug() << "Intermediate node didn't know about this transaction";
        return processNextNodeToCheckVotes();
    }

    if (kSenderUUID != mCurrentNodeToCheckVotes){
        warning() << "Sender is not current checking node";
        return resultContinuePreviousState();
    }

    mParticipantsVotesMessage = kMessage;
    mParticipantsSignatures = mParticipantsVotesMessage->participantsSignatures();

    return processParticipantsVotesMessage();
}

TransactionResult::SharedConst BasePaymentTransaction::processNextNodeToCheckVotes()
{
    debug() << "processNextNodeToCheckVotes";
    if (mNodesToCheckVotes.empty()) {
        debug() << "No nodes left to be asked. Sleep";
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

TransactionResult::SharedConst BasePaymentTransaction::runRollbackByOtherTransactionStage()
{
    debug() << "runRollbackByOtherTransactionStage";
    rollBack();
    return resultDone();
}

const TrustLineAmount BasePaymentTransaction::totalReservedAmount(
    AmountReservation::ReservationDirection reservationDirection) const
{
    TrustLineAmount totalAmount = 0;
    for (const auto &nodeUUIDAndReservations : mReservations) {
        for (const auto &pathIDAndReservation : nodeUUIDAndReservations.second) {
            if (pathIDAndReservation.second->direction() == reservationDirection) {
                totalAmount += pathIDAndReservation.second->amount();
            }
        }
    }
    return totalAmount;
}

pair<BytesShared, size_t> BasePaymentTransaction::getSerializedReceipt(
    const NodeUUID &source,
    const NodeUUID &target,
    const TrustLineAmount &amount)
{
    size_t serializedDataSize = NodeUUID::kBytesSize
                                + NodeUUID::kBytesSize
                                + TransactionUUID::kBytesSize
                                + kTrustLineAmountBytesCount
                                + sizeof(AuditNumber);
    BytesShared serializedData = tryMalloc(serializedDataSize);

    size_t bytesBufferOffset = 0;
    memcpy(
        serializedData.get() + bytesBufferOffset,
        source.data,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    memcpy(
        serializedData.get() + bytesBufferOffset,
        target.data,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

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
    if (source == mNodeUUID) {
        currentAuditNumber = mTrustLines->auditNumber(target);
    } else {
        currentAuditNumber = mTrustLines->auditNumber(source);
    }
    memcpy(
        serializedData.get() + bytesBufferOffset,
        &currentAuditNumber,
        sizeof(AuditNumber));

    return make_pair(
        serializedData,
        serializedDataSize);
}

bool BasePaymentTransaction::checkAllNeighborsWithReservationsAreInFinalParticipantsList()
{
    for (const auto &nodeAndReservations : mReservations) {
        if (mPaymentNodesIds.find(nodeAndReservations.first) == mPaymentNodesIds.end()) {
            return false;
        }
    }
    return true;
}

bool BasePaymentTransaction::checkAllPublicKeyHashesProperly()
{
    for (const auto &nodeUUIDAndPaymentNodeID : mPaymentNodesIds) {
        if (nodeUUIDAndPaymentNodeID.first == coordinatorUUID()) {
            continue;
        }
        if (mParticipantsPublicKeysHashes.find(nodeUUIDAndPaymentNodeID.first) ==
                mParticipantsPublicKeysHashes.end()) {
            warning() << "Public key hash of " << nodeUUIDAndPaymentNodeID.first << " is absent";
            return false;
        }
        if (mParticipantsPublicKeysHashes[nodeUUIDAndPaymentNodeID.first].first !=
                nodeUUIDAndPaymentNodeID.second) {
            warning() << "Invalid Payment node ID of " << nodeUUIDAndPaymentNodeID.first;
            return false;
        }
    }
}

const TrustLineAmount BasePaymentTransaction::totalReservedIncomingAmountToNode(
    const NodeUUID &nodeUUID)
{
    auto result = TrustLine::kZeroAmount();
    if (mReservations.find(nodeUUID) == mReservations.end()) {
        return result;
    }
    for (const auto &pathIDAndReservation : mReservations[nodeUUID]) {
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
    const NodeUUID &nodeUUID)
{
    size_t serializedDataSize = TransactionUUID::kBytesSize
                                + NodeUUID::kBytesSize
                                + sizeof(SerializedRecordsCount)
                                + mPaymentNodesIds.size() *
                                    (NodeUUID::kBytesSize + sizeof(PaymentNodeID));
    BytesShared serializedData = tryMalloc(serializedDataSize);

    size_t bytesBufferOffset = 0;
    memcpy(
        serializedData.get() + bytesBufferOffset,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize);
    bytesBufferOffset += TransactionUUID::kBytesSize;

    memcpy(
        serializedData.get() + bytesBufferOffset,
        nodeUUID.data,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    auto participantsCount = mPaymentNodesIds.size();
    memcpy(
        serializedData.get() + bytesBufferOffset,
        &participantsCount,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (const auto &nodeUUIDAndPaymentNodeID : mPaymentNodesIds) {
        memcpy(
            serializedData.get() + bytesBufferOffset,
            nodeUUIDAndPaymentNodeID.first.data,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;

        memcpy(
            serializedData.get() + bytesBufferOffset,
            &nodeUUIDAndPaymentNodeID.second,
            sizeof(PaymentNodeID));
        bytesBufferOffset += sizeof(PaymentNodeID);
    }

    return make_pair(
        serializedData,
        serializedDataSize);
}

bool BasePaymentTransaction::checkSignsAppropriate()
{
    if (mParticipantsSignatures.size() != mParticipantsPublicKeys.size()) {
        warning() << "different numbers of signatures and participants";
        return false;
    }
    for (const auto &nodeUUIDAndPaymentNodeID : mParticipantsPublicKeys) {
        if (mParticipantsSignatures.find(nodeUUIDAndPaymentNodeID.first) == mParticipantsSignatures.end()) {
            warning() << "there are no signature from node " << nodeUUIDAndPaymentNodeID.first
                      << " [" << nodeUUIDAndPaymentNodeID.first << "]";
        }
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
                        + mPaymentNodesIds.size()
                          * (NodeUUID::kBytesSize + sizeof(PaymentNodeID))
                        + mParticipantsPublicKeys.size()
                          * (sizeof(PaymentNodeID) + lamport::PublicKey::keySize());

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
        // Map key (NodeUUID)
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            nodeAndReservations.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;

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

            // TransactionUUID
            memcpy(
                dataBytesShared.get() + dataBytesOffset,
                mTransactionUUID.data,
                TransactionUUID::kBytesSize);
            dataBytesOffset += TransactionUUID::kBytesSize;

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
    auto kTotalParticipantsCount = mPaymentNodesIds.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &kTotalParticipantsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    // NodeUUIDs and payment IDs
    for (auto const &nodeUUIDAndPaymentNodeID : mPaymentNodesIds) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            nodeUUIDAndPaymentNodeID.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;

        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &nodeUUIDAndPaymentNodeID.second,
            sizeof(PaymentNodeID));
        dataBytesOffset += sizeof(PaymentNodeID);
    }

    // Payment IDs and publicKeys
    for (const auto &nodeIDAndPublicKey : mParticipantsPublicKeys) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &nodeIDAndPublicKey.first,
            sizeof(PaymentNodeID));
        dataBytesOffset += sizeof(PaymentNodeID);

        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            nodeIDAndPublicKey.second->data(),
            lamport::PublicKey::keySize());
        dataBytesOffset += lamport::PublicKey::keySize();
    }

    return make_pair(
        dataBytesShared,
        bytesCount);
}

size_t BasePaymentTransaction::reservationsSizeInBytes() const {
    size_t reservationSizeInBytes = 0;
    for (const auto &nodeAndReservations : mReservations){
        reservationSizeInBytes += NodeUUID::kBytesSize
                                  + nodeAndReservations.second.size() * (
                                    sizeof(PathID) + // PathID
                                    kTrustLineAmountBytesCount +  // Reservation Amount
                                    TransactionUUID::kBytesSize + // Reservation Transaction UUID
                                                                  // Reservation Direction
                                    sizeof(AmountReservation::SerializedReservationDirectionSize))
                                  + sizeof(SerializedRecordsCount); // Vector Size

    }
    reservationSizeInBytes += sizeof(SerializedRecordsCount); // map Size
    return reservationSizeInBytes;
}

const NodeUUID& BasePaymentTransaction::coordinatorUUID() const
{
    return currentNodeUUID();
}

const SerializedPathLengthSize BasePaymentTransaction::cycleLength() const
{
    return 0;
}

bool BasePaymentTransaction::isCommonVotesCheckingStage() const
{
    return mStep == Common_VotesChecking;
}

void BasePaymentTransaction::setRollbackByOtherTransactionStage()
{
    mStep = Common_RollbackByOtherTransaction;
}

void BasePaymentTransaction::runThreeNodesCyclesTransactions() {
    mBuildCycleThreeNodesSignal(
        mCreditorsForCycles,
        mEquivalent);
}

void BasePaymentTransaction::runFourNodesCyclesTransactions() {
    mBuildCycleFourNodesSignal(
        mCreditorsForCycles,
        mEquivalent);
}

