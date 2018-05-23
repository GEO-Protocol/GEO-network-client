#include "BasePaymentTransaction.h"

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const NodeUUID &currentNodeUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        type,
        currentNodeUUID,
        equivalent,
        log),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr),
    mCoordinatorAlreadySentFinalAmountsConfiguration(false)
{}

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &currentNodeUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        type,
        transactionUUID,
        currentNodeUUID,
        equivalent,
        log),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr),
    mCoordinatorAlreadySentFinalAmountsConfiguration(false)
{}

BasePaymentTransaction::BasePaymentTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        buffer,
        nodeUUID,
        log),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController)
{
    auto bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    mStep = Stages::Common_Recovery;

    // mEquivalent
    memcpy(
        &mEquivalent,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedEquivalent));
    bytesBufferOffset += sizeof(SerializedEquivalent);

    // mParticipantsVotesMessage
    size_t participantsVotesMessageBytesCount;
    memcpy(
        &participantsVotesMessageBytesCount,
        buffer.get() + bytesBufferOffset,
        sizeof(size_t));
    bytesBufferOffset += sizeof(size_t);

    BytesShared participantsVotesMessageBytes = tryMalloc(participantsVotesMessageBytesCount);
    memcpy(
        participantsVotesMessageBytes.get(),
        buffer.get() + bytesBufferOffset,
        participantsVotesMessageBytesCount);
    bytesBufferOffset += participantsVotesMessageBytesCount;

    mParticipantsVotesMessage = make_shared<ParticipantsVotesMessage>(
        participantsVotesMessageBytes);

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
            stepVector.push_back(
                make_pair(
                    stepPathID,
                    stepAmountReservation));

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
}

/*
 * Handles votes list message receiving or it's absence.
 * Controls transaction approving/rejecting/rolling back.
 */
TransactionResult::SharedConst BasePaymentTransaction::runVotesCheckingStage()
{
    debug() << "runVotesCheckingStage";
    // Votes message may be received twice:
    // First time - as a request to check the transaction and to sing it in case if all correct.
    // Second time - as a command to commit/rollback the transaction.
    if (mTransactionIsVoted)
        return runVotesConsistencyCheckingStage();


    if (! contextIsValid(Message::Payments_ParticipantsVotes))
        return reject("No participants votes received. Canceling.");

    if (!checkReservationsDirections()) {
        return reject("Reservations on node are invalid");
    }

    const auto kCurrentNodeUUID = currentNodeUUID();
    mParticipantsVotesMessage = popNextMessage<ParticipantsVotesMessage>();
    debug() << "Votes message received";

    if (!checkAllNeighborsPresence()) {
        return reject("Some neighbors which are involved in transaction are absent in participants");
    }

    try {
        // Check if current node is listed in the votes list.
        // This check is needed to prevent processing message in case of missdelivering.
        mParticipantsVotesMessage->vote(kCurrentNodeUUID);

    } catch (NotFoundError &) {
        // It seems that current node wasn't listed in the votes list.
        // This is possible only in case, when one node takes part in 2 parallel transactions,
        // that have common UUID (transactions UUIDs collision).
        // The probability of this is very small, but is present.
        //
        // In this case - the message must be simply ignored.

        warning() << "Votes message ignored due to transactions UUIDs collision detected.";
        debug() << "Waiting for another votes message.";

        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(
                mParticipantsVotesMessage->participantsCount())); // ToDo: kMessage->participantsCount() must not be used (it is invalid)
    }

    if (mParticipantsVotesMessage->containsRejectVote()) {
        // Some node rejected the transaction.
        // This node must simply roll back it's part of transaction and exit.
        // No further message propagation is needed.
        return reject("Some participant node has been rejected the transaction. Rolling back.");
    }

    // TODO : insert propagate message here
    mParticipantsVotesMessage->approve(kCurrentNodeUUID);
    mTransactionIsVoted = true;
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

    debug() << "Voted +";

#ifdef TESTS
    mSubsystemsController->testForbidSendMessageToNextNodeOnVoteStage();
    mSubsystemsController->testThrowExceptionOnVoteStage();
    mSubsystemsController->testTerminateProcessOnVoteStage();
#endif

    try {
        // Try to get next participant from the message.
        // In case if this node is the last node in votes list -
        // then it must be propagated to all nodes as successfully signed transaction.
        const auto kNextParticipant = mParticipantsVotesMessage->nextParticipant(kCurrentNodeUUID);
        const auto kNewParticipantsVotesMessage  = make_shared<ParticipantsVotesMessage>(
            mNodeUUID,
            mParticipantsVotesMessage);

#ifdef TESTS
        // node wait for vote message after final amounts confirmation maxNetworkDelay(5)
        mSubsystemsController->testSleepOnOnVoteStage(maxNetworkDelay(6));
#endif

        // NotFoundError wasn't thrown.
        // Current node is not last in the votes list.
        // Message must be transferred to the next node in the list.
        sendMessage(
            kNextParticipant,
            kNewParticipantsVotesMessage);

        debug() << "Votes list message transferred to the (" << kNextParticipant << ")";

        mStep = Stages::Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(
                mParticipantsVotesMessage->participantsCount() + 1));

    } catch (NotFoundError &) {
        // There are no nodes left in the votes list.
        // Current node is the last node that has signed the transaction.
        // Now it must be transferred to coordinator
        // and then propagate to all nodes in the votes list
        // as successfully signed transaction.

#ifdef TESTS
        mSubsystemsController->testForbidSendMessageToCoordinatorOnVoteStage();
        // coordinator wait for message with all votes
        // maxNetworkDelay(mParticipantsVotesMessage->participantsCount() + 1)
        mSubsystemsController->testSleepOnVoteConsistencyStage(
            maxNetworkDelay(
                mParticipantsVotesMessage->participantsCount() + 2));
#endif

        const auto kNewParticipantsVotesMessage  = make_shared<ParticipantsVotesMessage>(
            mNodeUUID,
            mParticipantsVotesMessage);
        debug() << "Votes list transferred to coordinator " << kNewParticipantsVotesMessage->coordinatorUUID();
        sendMessage(
            kNewParticipantsVotesMessage->coordinatorUUID(),
            kNewParticipantsVotesMessage);

        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(3));
    }
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

    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    debug () << "Participants votes message received.";

    if (!checkOldAndNewParticipants(kMessage)) {
        return reject("Participants votes message is invalid. Rolling back.");
    }

    mParticipantsVotesMessage = kMessage;

    if (mParticipantsVotesMessage->containsRejectVote()) {
        return reject("Some participant node has been rejected the transaction. Rolling back.");
    }

    // Checking if no one node has been deleted current nodes sign.
    // (Message modification protection)
    // ToDo: [mvp+] add cryptographic mechanism to prevent removing of votes.
    if (!positiveVoteIsPresent(mParticipantsVotesMessage)) {
        // Note: there is no correct way to exit from the transaction
        // in case if some one removed the vote from the message.
        // Rolling transaction back only reverts current node.

        rollBack();
        throw RuntimeError(
                "BasePaymentTransaction::runVotesConsistencyCheckingStage: "
                        "Some node has been modified the message and removed the vote of the current node.");
    }

    if (mParticipantsVotesMessage->achievedConsensus()){
        // In case if votes message received again -
        debug() << "Votes list received. Consensus achieved.";
        return approve();

    } else {
        // Otherwise - message contains some uncertain votes.
        // In this case - message may be ignored.
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(
                mParticipantsVotesMessage->participantsCount()));
        }
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

        mReservations[neighborNode].push_back(
            make_pair(
                pathID,
                kReservation));
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

        mReservations[neighborNode].push_back(
            make_pair(
                pathID,
                kReservation));
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
    if (message)
        warning() << message;

    // Participants votes may not be received,
    // if transaction doesn't achieved votes processing state yet.
    if (mParticipantsVotesMessage != nullptr) {
        if (currentNodeUUID() == mParticipantsVotesMessage->coordinatorUUID()) {
            mParticipantsVotesMessage->reject(mParticipantsVotesMessage->firstParticipant());
        } else {
            mParticipantsVotesMessage->reject(currentNodeUUID());
        }
        auto ioTransaction = mStorageHandler->beginTransaction();
        saveVotes(ioTransaction);
        const auto kNewParticipantsVotesMessage  = make_shared<ParticipantsVotesMessage>(
            mNodeUUID,
            mParticipantsVotesMessage);
        propagateVotesMessageToAllParticipants(kNewParticipantsVotesMessage);
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
        for (const auto &kPathIDAndReservation : kNodeUUIDAndReservations.second) {
            mTrustLines->useReservation(kNodeUUIDAndReservations.first, kPathIDAndReservation.second);

            if (kPathIDAndReservation.second->direction() == AmountReservation::Outgoing) {
                debug() << "Committed reservation: [ => ] " << kPathIDAndReservation.second->amount()
                        << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathIDAndReservation.first
                        << "]";
                mCreditorsForCycles.insert(kNodeUUIDAndReservations.first);
            }
            else if (kPathIDAndReservation.second->direction() == AmountReservation::Incoming)
                debug() << "Committed reservation: [ <= ] " << kPathIDAndReservation.second->amount()
                        << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathIDAndReservation.first
                        << "]";

            mTrustLines->dropAmountReservation(
                kNodeUUIDAndReservations.first,
                kPathIDAndReservation.second);
        }
        if (mTrustLines->isTrustLineEmpty(kNodeUUIDAndReservations.first)) {
            mTrustLines->removeTrustLine(
                ioTransaction,
                kNodeUUIDAndReservations.first);
        } else ioTransaction->trustLinesHandler()->saveTrustLine(
                mTrustLines->trustLines().at(
                    kNodeUUIDAndReservations.first),
                mEquivalent);
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
    const auto kNewParticipantsVotesMessage  = make_shared<ParticipantsVotesMessage>(
        mNodeUUID,
        mParticipantsVotesMessage);
    auto bufferAndSize = kNewParticipantsVotesMessage->serializeToBytes();
    ioTransaction->paymentOperationStateHandler()->saveRecord(
        mParticipantsVotesMessage->transactionUUID(),
        bufferAndSize.first,
        bufferAndSize.second);
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

TransactionResult::SharedConst BasePaymentTransaction::recover (
    const char *message)
{
    debug() << "recover";
    if (message != nullptr)
        warning() << message;

    if(mTransactionIsVoted and (mParticipantsVotesMessage != nullptr)){
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

/**
 * @returns true in case if "kMessage" contains positive vote for the transaction.
 * Otherwise - returns false.
 */
const bool BasePaymentTransaction::positiveVoteIsPresent (
    const ParticipantsVotesMessage::ConstShared kMessage) const
{
    // ToDo: add cryptographic mechanism to prevent removing of votes.

    try {
        const auto kCurrentNodeVote = kMessage->vote(currentNodeUUID());
        if (kCurrentNodeVote == ParticipantsVotesMessage::Approved)
            return true;

    } catch (NotFoundError &){}

    return false;
}

void BasePaymentTransaction::propagateVotesMessageToAllParticipants (
    const ParticipantsVotesMessage::Shared kMessage) const
{
    debug() << "propagateVotesMessageToAllParticipants";
    const auto kCurrentNodeUUID = currentNodeUUID();

    auto participant = kMessage->firstParticipant();
    if (participant != kCurrentNodeUUID) {
        sendMessage(
            participant,
            kMessage);
    }

    for (;;) {
        try {
            participant = kMessage->nextParticipant(participant);
            if (participant != kCurrentNodeUUID) {
                sendMessage(
                    participant,
                    kMessage);
            }

        } catch (NotFoundError &) {
            break;
        }
    }

    if (kCurrentNodeUUID != kMessage->coordinatorUUID()) {
        // Sending votes list to the coordinator,
        // so it will be able to commit the transaction.
        sendMessage(
            kMessage->coordinatorUUID(),
            kMessage);
    }
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
        mReservations[kContractor].push_back(
            make_pair(
                pathID,
                updatedReservation));

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

bool BasePaymentTransaction::checkAllNeighborsPresence() const
{
    debug() << "checkAllNeighborsPresence";
    for (const auto &reservation : mReservations) {
        if (reservation.first == coordinatorUUID()) {
            continue;
        }
        if (!mParticipantsVotesMessage->containsParticipant(reservation.first)) {
            return false;
        }
    }
    debug() << "All neighbors are present in participants votes message";
    return true;
}

bool BasePaymentTransaction::checkOldAndNewParticipants(
    ParticipantsVotesMessage::Shared newMessageWithVotes,
    bool checkCoordinatorPresence)
{
    debug() << "checkOldAndNewParticipants";
    if (checkCoordinatorPresence) {
        if (!newMessageWithVotes->containsParticipant(coordinatorUUID())) {
            warning() << "New message doesn't contain coordinator vote";
            return false;
        }
        if (newMessageWithVotes->participantsCount() != mParticipantsVotesMessage->participantsCount() + 1) {
            warning() << "Wrong participants count in new message";
            return false;
        }
    } else {
        if (newMessageWithVotes->participantsCount() != mParticipantsVotesMessage->participantsCount()) {
            warning() << "Wrong participants count in new message";
            return false;
        }
    }
    for (const auto &oldParticipant : mParticipantsVotesMessage->votes()) {
        if (!newMessageWithVotes->containsParticipant(oldParticipant.first)) {
            warning() << "Different participants in new and old messages";
            return false;
        }
    }
    return true;
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
    const auto kCoordinatorUUID = mParticipantsVotesMessage->coordinatorUUID();
    for(const auto &kNodeUUIDAndVote: mParticipantsVotesMessage->votes()){
        if (kNodeUUIDAndVote.first != kCoordinatorUUID and kNodeUUIDAndVote.first != mNodeUUID)
            mNodesToCheckVotes.push_back(kNodeUUIDAndVote.first);
    }
    if(kCoordinatorUUID == mNodeUUID) {
        mVotesRecoveryStep = VotesRecoveryStages::Common_CheckIntermediateNodeVotesStage;
        mCurrentNodeToCheckVotes = mNodesToCheckVotes.back();
        mNodesToCheckVotes.pop_back();
        return sendVotesRequestMessageAndWaitForResponse(mCurrentNodeToCheckVotes);
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
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(2));
    }

    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    const auto kCoordinatorUUID = kMessage->coordinatorUUID();
    const auto kSenderUUID = kMessage->senderUUID;

    if (kCoordinatorUUID == NodeUUID::empty() || kMessage->votes().empty()) {
        debug() << "Coordinator don't know result of this transaction yet. Sleep.";
        mVotesRecoveryStep = VotesRecoveryStages::Common_PrepareNodesListToCheckVotes;
        return resultAwakeAfterMilliseconds(
            kWaitMillisecondsToTryRecoverAgain);
    }

    // Check if answer is from Coordinator
    if (kSenderUUID != kCoordinatorUUID){
        warning() << "Sender (" << kSenderUUID << ") is not coordinator ("
                << kCoordinatorUUID << "), ignore this message";
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(2));
    }

    if (kMessage->containsRejectVote()) {
        mParticipantsVotesMessage = kMessage;
        return reject("ParticipantsVotesMessage contains reject. Rejecting");
    }

    if (kMessage->achievedConsensus()){
        mParticipantsVotesMessage = kMessage;
        debug() << "Achieved consensus";
        return approve();
    }

    // TODO : need discuss this case. it can't be happen
    warning() << "Unexpected behaviour. Apply logic when coordinator didn't sent response";
    return processNextNodeToCheckVotes();
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
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(2));
    }

    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    const auto kSenderUUID = kMessage->senderUUID;

    if (kMessage->votes().empty()) {
        debug() << "Intermediate node didn't know about this transaction";
        return processNextNodeToCheckVotes();
    }

    if (kSenderUUID != mCurrentNodeToCheckVotes){
        warning() << "Sender is not current checking node";
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(2));
    }

    if (kMessage->containsRejectVote()) {
        mParticipantsVotesMessage = kMessage;
        return reject("ParticipantsVotesMessage contains reject. Rejecting");
    }
    if (kMessage->achievedConsensus()) {
        mParticipantsVotesMessage = kMessage;
        debug() << "Achieved consensus";
        return approve();
    }

    debug() << "Unknown status of transaction";
    return processNextNodeToCheckVotes();
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

bool BasePaymentTransaction::compareReservations(
    const vector<pair<PathID, AmountReservation::ConstShared>> &localReservations,
    const vector<pair<PathID, AmountReservation::ConstShared>> &remoteReservations)
{
    if (localReservations.size() != remoteReservations.size()) {
        return false;
    }
    for (const auto &localPathAndReservation : localReservations) {
        bool findAppropriateReservation = false;
        for (const auto &remotePathAndReservation : remoteReservations) {
            if (remotePathAndReservation.first == localPathAndReservation.first) {
                if (remotePathAndReservation.second->amount() == localPathAndReservation.second->amount() and
                    remotePathAndReservation.second->direction() != localPathAndReservation.second->direction()) {
                    findAppropriateReservation = true;
                    break;
                } else {
                    return false;
                }
            }
        }
        if (!findAppropriateReservation) {
            return false;
        }
    }
    return true;
}

bool BasePaymentTransaction::checkAllNeighborsReservationsAppropriate()
{
    for (const auto &nodeAndReservations : mReservations) {
        if (mRemoteReservations.find(nodeAndReservations.first) == mRemoteReservations.end()) {
            return false;
        }
        const auto remoteReservations = mRemoteReservations[nodeAndReservations.first];
        if (!compareReservations(nodeAndReservations.second, remoteReservations)) {
            return false;
        }
    }
    return true;
}

pair<BytesShared, size_t> BasePaymentTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    // mParticipantsVotesMessage Part
    const auto kBufferAndSizeParticipantsVotesMessage = mParticipantsVotesMessage->serializeToBytes();
    // parent part
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedEquivalent)
                        + sizeof(size_t)
                        + kBufferAndSizeParticipantsVotesMessage.second
                        + reservationsSizeInBytes();

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    // Parent part
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    // mEquivalent
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mEquivalent,
        sizeof(SerializedEquivalent));
    dataBytesOffset += sizeof(SerializedEquivalent);

    // mParticipantsVotesMessage Part
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &kBufferAndSizeParticipantsVotesMessage.second,
        sizeof(size_t));
    dataBytesOffset += sizeof(size_t);

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        kBufferAndSizeParticipantsVotesMessage.first.get(),
        kBufferAndSizeParticipantsVotesMessage.second);
    dataBytesOffset += kBufferAndSizeParticipantsVotesMessage.second;

    // Reservation Part
    SerializedRecordsCount kmReservationSize = (SerializedRecordsCount)mReservations.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &kmReservationSize,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for(auto it=mReservations.begin(); it!=mReservations.end(); it++){
        // Map key (NodeUUID)
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &it->first,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;

        // Size of map value vector
        SerializedRecordsCount kReservationsValueSize = (SerializedRecordsCount)it->second.size();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &kReservationsValueSize,
            sizeof(SerializedRecordsCount));
        dataBytesOffset += sizeof(SerializedRecordsCount);

        for(const auto &kReservationValues: it->second){
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
    return make_pair(
        dataBytesShared,
        bytesCount);
}

size_t BasePaymentTransaction::reservationsSizeInBytes() const {
    size_t reservationSizeInBytes = 0;
    for (auto it=mReservations.begin(); it!=mReservations.end(); it++){
        reservationSizeInBytes += NodeUUID::kBytesSize + (
                                sizeof(PathID) + // PathID
                                kTrustLineAmountBytesCount +  // Reservation Amount
                                TransactionUUID::kBytesSize + // Reservation Transaction UUID
                                                              // Reservation Direction
                                sizeof(AmountReservation::SerializedReservationDirectionSize)) * it->second.size() +
                                sizeof(SerializedRecordsCount); // Vector Size

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

