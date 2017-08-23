#include "BasePaymentTransaction.h"

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const NodeUUID &currentNodeUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        type,
        currentNodeUUID,
        log),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mSubsystemsController(subsystemsController),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr)
{}

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &currentNodeUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        type,
        transactionUUID,
        currentNodeUUID,
        log),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mSubsystemsController(subsystemsController),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr)
{}

BasePaymentTransaction::BasePaymentTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &log,
    SubsystemsController *subsystemsController) :

    BaseTransaction(
        buffer,
        nodeUUID,
        log),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mSubsystemsController(subsystemsController)
{
    auto bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    mStep = Stages::Common_Recovery;

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
    uint64_t reservationsCount;
    memcpy(
        &reservationsCount,
        buffer.get() + bytesBufferOffset,
        sizeof(uint64_t));
    bytesBufferOffset += sizeof(uint64_t);

    // Map values
    for(auto i=1; i<=reservationsCount; i++){
        // Map Key NodeUUID
        NodeUUID stepNodeUUID;
        memcpy(
            &stepNodeUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;

        // Map values vector
        uint64_t stepReservationVectorSize;
        memcpy(
            &stepReservationVectorSize,
            buffer.get() + bytesBufferOffset,
            sizeof(uint64_t));
        bytesBufferOffset += sizeof(uint64_t);

        vector<pair<PathUUID, AmountReservation::ConstShared>> stepVector;
        for(auto j=1; j<=stepReservationVectorSize; j++){

            // PathUUID
            PathUUID stepPathUUID;
            memcpy(
                &stepPathUUID,
                buffer.get() + bytesBufferOffset,
                sizeof(PathUUID));
            bytesBufferOffset += sizeof(PathUUID);

            // Amount
            TrustLineAmount stepAmount;
            vector<byte> amountBytes(
                buffer.get() + bytesBufferOffset,
                buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
            stepAmount = bytesToTrustLineAmount(amountBytes);
            bytesBufferOffset += kTrustLineAmountBytesCount;

            // Transaction UUID
            TransactionUUID stepTransactionUUID;
            memcpy(
                &stepTransactionUUID.data,
                buffer.get() + bytesBufferOffset,
                TransactionUUID::kBytesSize);
            bytesBufferOffset += TransactionUUID::kBytesSize;

            // Direction
            uint8_t stepDirection;
            memcpy(
                &stepDirection,
                buffer.get() + bytesBufferOffset,
                sizeof(uint8_t));
            bytesBufferOffset += sizeof(uint8_t);
            auto stepEnumDirection = static_cast<AmountReservation::ReservationDirection>(stepDirection);

            auto stepAmountReservation = make_shared<AmountReservation>(
                stepTransactionUUID,
                stepAmount,
                stepEnumDirection);
            stepVector.push_back(
                make_pair(
                    stepPathUUID,
                    stepAmountReservation));

            if (stepDirection == AmountReservation::ReservationDirection::Incoming) {
                if (!reserveIncomingAmount(
                    stepNodeUUID,
                    stepAmount,
                    stepPathUUID)) {
                    // can't create reserve, but this reserve was serialized before node dropping
                    // we must stop this node and find out the reason
                    exit(1);
                }
            }

            if (stepDirection == AmountReservation::ReservationDirection::Outgoing) {
                if (!reserveOutgoingAmount(
                    stepNodeUUID,
                    stepAmount,
                    stepPathUUID)) {
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

        debug() << "Votes message ignored due to transactions UUIDs collision detected.";
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
    // TODO: flush
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

    mParticipantsVotesMessage = kMessage;
    if (mParticipantsVotesMessage->containsRejectVote()) {
        return reject("Some participant node has been rejected the transaction. Rolling back.");
    }

    // Checking if no one node has been deleted current nodes sign.
    // (Message modification protection)
    // ToDo: [mvp+] add cryptographic mechanism to prevent removing of votes.
    if (currentNodeUUID() != mParticipantsVotesMessage->coordinatorUUID()) {
        if (!positiveVoteIsPresent(mParticipantsVotesMessage)) {
            // Note: there is no correct way to exit from the transaction
            // in case if some one removed the vote from the message.
            // Rolling transaction back only reverts current node.

            rollBack();
            throw RuntimeError(
                    "BasePaymentTransaction::runVotesConsistencyCheckingStage: "
                            "Some node has been modified the message and removed the vote of the current node.");
        }
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
    const PathUUID &pathUUID)
{
    try {
        const auto kReservation = mTrustLines->reserveOutgoingAmount(
            neighborNode,
            currentTransactionUUID(),
            amount);

#ifdef DEBUG
        // todo: uncomment me, when problem with recoverin transaction and reservation after restarting node will be fixed
        //debug() << "Reserved " << amount << " for (" << neighborNode << ") [" << pathUUID << "] [Outgoing amount reservation].";
#endif

        mReservations[neighborNode].push_back(
            make_pair(
                pathUUID,
                kReservation));
        return true;

    } catch (Exception &) {}

    return false;
}

const bool BasePaymentTransaction::reserveIncomingAmount(
    const NodeUUID& neighborNode,
    const TrustLineAmount& amount,
    const PathUUID &pathUUID)
{
    try {
        const auto kReservation = mTrustLines->reserveIncomingAmount(
            neighborNode,
            currentTransactionUUID(),
            amount);

#ifdef DEBUG
        // todo: uncomment me, when problem with recoverin transaction and reservation after restarting node will be fixed
        //debug() << "Reserved " << amount << " for (" << neighborNode << ") [" << pathUUID << "] [Incoming amount reservation].";
#endif

        mReservations[neighborNode].push_back(
            make_pair(
                pathUUID,
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
            error() << "contextIsValid::context is empty";
        }
        return false;
    }

    if (mContext.size() > 1) {
        if (showErrorMessage) {
            stringstream stream;
            stream << "contextIsValid::context has " << mContext.size() << " messages: ";
            for (auto const message : mContext) {
                stream << message->typeID() << " ";
            }
            error() << stream.str();
        }
        return false;
    }

    if (mContext.at(0)->typeID() != messageType) {
        if (showErrorMessage) {
            error() << "Unexpected message received. (ID " << mContext.at(0)->typeID()
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
        info() << message;

    // Participants votes may not be received,
    // if transaction doesn't achieved votes processing state yet.
    if (mParticipantsVotesMessage != nullptr) {
        if (currentNodeUUID() == mParticipantsVotesMessage->coordinatorUUID()) {
            mParticipantsVotesMessage->reject(mParticipantsVotesMessage->firstParticipant());
        } else {
            mParticipantsVotesMessage->reject(currentNodeUUID());
        }
        saveVotes();
        const auto kNewParticipantsVotesMessage  = make_shared<ParticipantsVotesMessage>(
            mNodeUUID,
            mParticipantsVotesMessage);
        propagateVotesMessageToAllParticipants(kNewParticipantsVotesMessage);
    }

    rollBack();
    debug() << "Transaction successfully rolled back.";

    return resultDone();
}

TransactionResult::SharedConst BasePaymentTransaction::cancel(
    const char *message)
{
    if (message)
        info() << message;

    // Participants votes may not be received,
    // if transaction doesn't achieved votes processing state yet.

    rollBack();
    info() << "Transaction succesfully rolled back.";

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
    commit();
    savePaymentOperationIntoHistory();
    return resultDone();
}

void BasePaymentTransaction::commit ()
{
    debug() << "Transaction committing...";

//    // TODO: Ensure atomicity in case if some reservations would be used, and transaction crash.
//    {
//        const auto ioTransaction = mStorageHandler->beginTransaction();
//    }

    {
        // TODO : discuss using of ioTransaction and saveVotes()
        auto ioTransaction = mStorageHandler->beginTransaction();
        for (const auto &kNodeUUIDAndReservations : mReservations) {
            for (const auto &kPathUUIDAndReservation : kNodeUUIDAndReservations.second) {
                mTrustLines->useReservation(kNodeUUIDAndReservations.first, kPathUUIDAndReservation.second);

                if (kPathUUIDAndReservation.second->direction() == AmountReservation::Outgoing)
                    debug() << "Committed reservation: [ => ] " << kPathUUIDAndReservation.second->amount()
                            << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathUUIDAndReservation.first
                            << "]";

                else if (kPathUUIDAndReservation.second->direction() == AmountReservation::Incoming)
                    debug() << "Committed reservation: [ <= ] " << kPathUUIDAndReservation.second->amount()
                            << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathUUIDAndReservation.first
                            << "]";
                mTrustLines->dropAmountReservation(kNodeUUIDAndReservations.first, kPathUUIDAndReservation.second);
            }
            ioTransaction->trustLineHandler()->saveTrustLine(
                mTrustLines->trustLines().at(kNodeUUIDAndReservations.first));
        }
    }

    // delete transaction references on dropped reservations
    mReservations.clear();

    // reset initiator cashe, becouse after changing balanses
    // we need updated information on max flow calculation transaction
    mMaxFlowCalculationCacheManager->resetInititorCache();
    debug() << "Transaction committed.";
    saveVotes();
    debug() << "Votes saved.";
    // TODO: Ensure atomicity in case if some reservations would be used, and transaction crash.
    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->transactionHandler()->deleteRecord(currentTransactionUUID());
    }
}

void BasePaymentTransaction::saveVotes()
{
    debug() << "saveVotes";
    const auto ioTransaction = mStorageHandler->beginTransaction();
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
    // drop reservations in AmountReservationHandler
    for (const auto &kNodeUUIDAndReservations : mReservations) {
        for (const auto &kPathUUIDAndReservation : kNodeUUIDAndReservations.second) {
            mTrustLines->dropAmountReservation(kNodeUUIDAndReservations.first, kPathUUIDAndReservation.second);

            if (kPathUUIDAndReservation.second->direction() == AmountReservation::Outgoing)
                debug() << "Dropping reservation: [ => ] " << kPathUUIDAndReservation.second->amount()
                        << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathUUIDAndReservation.first << "]";

            else if (kPathUUIDAndReservation.second->direction() == AmountReservation::Incoming)
                debug() << "Dropping reservation: [ <= ] " << kPathUUIDAndReservation.second->amount()
                        << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathUUIDAndReservation.first << "]";
        }
    }

    // delete transaction references on dropped reservations
    mReservations.clear();

    {
        const auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->transactionHandler()->deleteRecord(currentTransactionUUID());
    }
}

void BasePaymentTransaction::rollBack (
    const PathUUID &pathUUID)
{
    debug() << "rollback on path";
    auto itNodeUUIDAndReservations = mReservations.begin();
    while(itNodeUUIDAndReservations != mReservations.end()) {
        auto itPathUUIDAndReservation = itNodeUUIDAndReservations->second.begin();
        while (itPathUUIDAndReservation != itNodeUUIDAndReservations->second.end()) {
            if (itPathUUIDAndReservation->first == pathUUID) {
                mTrustLines->dropAmountReservation(
                    itNodeUUIDAndReservations->first,
                    itPathUUIDAndReservation->second);

                if (itPathUUIDAndReservation->second->direction() == AmountReservation::Outgoing)
                    debug() << "Dropping reservation: [ => ] " << itPathUUIDAndReservation->second->amount()
                           << " for (" << itNodeUUIDAndReservations->first << ") [" << itPathUUIDAndReservation->first
                           << "]";

                else if (itPathUUIDAndReservation->second->direction() == AmountReservation::Incoming)
                    debug() << "Dropping reservation: [ <= ] " << itPathUUIDAndReservation->second->amount()
                           << " for (" << itNodeUUIDAndReservations->first << ") [" << itPathUUIDAndReservation->first
                           << "]";

                itPathUUIDAndReservation = itNodeUUIDAndReservations->second.erase(itPathUUIDAndReservation);
                } else {
                    itPathUUIDAndReservation++;
                }
            }
        if (itNodeUUIDAndReservations->second.size() == 0) {
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
        info() << message;

    if(mTransactionIsVoted and (mParticipantsVotesMessage != nullptr)){
        mStep = Stages::Common_Recovery;
        mVotesRecoveryStep = VotesRecoveryStages::Common_PrepareNodesListToCheckVotes;
        clearContext();
        return runVotesRecoveryParentStage();
    } else {
        debug() << "Transaction doesn't sent/receive participants votes message and will be closed";
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

uint32_t BasePaymentTransaction::maxCoordinatorResponseTimeout () const
{
    return maxNetworkDelay(1);
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

    // Sending votes list to the coordinator,
    // so it will be able to commit the transaction.
    sendMessage(
        kMessage->coordinatorUUID(),
        kMessage);
}

const bool BasePaymentTransaction::shortageReservation (
    const NodeUUID kContractor,
    const AmountReservation::ConstShared kReservation,
    const TrustLineAmount &kNewAmount,
    const PathUUID &pathUUID)
{
    debug() << "shortageReservation on path " << pathUUID;
    if (kNewAmount > kReservation->amount()) {
        throw ValueError(
                "BasePaymentTransaction::shortageReservation: "
                        "new amount can't be greater than already reserved one.");
    }

    try {
//#ifdef DEBUG
        const auto kPreviousAmount = kReservation->amount();
//#endif

        auto updatedReservation = mTrustLines->updateAmountReservation(
            kContractor,
            kReservation,
            kNewAmount);

        for (auto it = mReservations[kContractor].begin(); it != mReservations[kContractor].end(); it++){
            // TODO detailed check this condition
            if ((*it).second.get() == kReservation.get() && (*it).first == pathUUID) {
                mReservations[kContractor].erase(it);
                break;
            }
        }
        mReservations[kContractor].push_back(
            make_pair(
                pathUUID,
                updatedReservation));

//#ifdef DEBUG
        if (kReservation->direction() == AmountReservation::Incoming)
            debug() << "Reservation for (" << kContractor << ") [" << pathUUID << "] shortened "
                   << "from " << kPreviousAmount << " to " << kNewAmount << " [<=]";
        else
            debug() << "Reservation for (" << kContractor << ") [" << pathUUID << "] shortened "
                   << "from " << kPreviousAmount << " to " << kNewAmount << " [=>]";
//#endif

        return true;

    } catch (NotFoundError &) {}

    return false;
}

TransactionResult::SharedConst BasePaymentTransaction::exitWithResult(
    TransactionResult::SharedConst result,
    const char *message)
{
    if (message != nullptr)
        info() << message;

    return result;
}

void BasePaymentTransaction::dropNodeReservationsOnPath(
    PathUUID pathUUID)
{
    debug() << "dropNodeReservationsOnPath: " << pathUUID;

    for (auto nodeReservations : mReservations) {
        //auto nodeReservations = mReservations.find(firstIntermediateNode);
        auto itPathUUIDAndReservation = nodeReservations.second.begin();
        while (itPathUUIDAndReservation != nodeReservations.second.end()) {
            if (itPathUUIDAndReservation->first == pathUUID) {
                mTrustLines->dropAmountReservation(
                    nodeReservations.first,
                    itPathUUIDAndReservation->second);

                if (itPathUUIDAndReservation->second->direction() == AmountReservation::Outgoing)
                    debug() << "Dropping reservation: [ => ] " << itPathUUIDAndReservation->second->amount()
                            << " for (" << nodeReservations.first << ") [" << itPathUUIDAndReservation->first
                            << "]";

                else if (itPathUUIDAndReservation->second->direction() == AmountReservation::Incoming)
                    debug() << "Dropping reservation: [ <= ] " << itPathUUIDAndReservation->second->amount()
                            << " for (" << nodeReservations.first << ") [" << itPathUUIDAndReservation->first
                            << "]";

                itPathUUIDAndReservation = nodeReservations.second.erase(itPathUUIDAndReservation);
            } else {
                itPathUUIDAndReservation++;
            }
        }
        if (nodeReservations.second.size() == 0) {
            mReservations.erase(nodeReservations.first);
        }
    }
}

void BasePaymentTransaction::sendFinalPathConfiguration(
    PathStats* pathStats,
    PathUUID pathUUID,
    const TrustLineAmount &finalPathAmount)
{
    debug() << "sendFinalPathConfiguration";
    for (const auto &intermediateNode : pathStats->path()->intermediateUUIDs()) {
        debug() << "send message with final path amount info for node " << intermediateNode;
        sendMessage<FinalPathConfigurationMessage>(
            intermediateNode,
            currentNodeUUID(),
            currentTransactionUUID(),
            pathUUID,
            finalPathAmount);
    }
}

bool BasePaymentTransaction::updateReservations(
    const vector<pair<PathUUID, ConstSharedTrustLineAmount>> &finalAmounts)
{
    unordered_set<PathUUID> updatedPaths;
    const auto reservationsCopy = mReservations;
    for (const auto &nodeAndReservations : reservationsCopy) {
        for (auto pathUUIDAndReservation : nodeAndReservations.second) {
            const auto updatedPathUUID = updateReservation(
                nodeAndReservations.first,
                pathUUIDAndReservation,
                finalAmounts);
            if (updatedPathUUID != UINT64_MAX) {
                updatedPaths.insert(updatedPathUUID);
            }
        }
    }
    return updatedPaths.size() == finalAmounts.size();
}

BasePaymentTransaction::PathUUID BasePaymentTransaction::updateReservation(
    const NodeUUID &contractorUUID,
    pair<PathUUID, AmountReservation::ConstShared> &reservation,
    const vector<pair<PathUUID, ConstSharedTrustLineAmount>> &finalAmounts)
{
    for (auto pathUUIDAndAmount : finalAmounts) {
        if (pathUUIDAndAmount.first == reservation.first) {
            // todo : maybe add if reservations are different
            shortageReservation(
                contractorUUID,
                reservation.second,
                *pathUUIDAndAmount.second.get(),
                pathUUIDAndAmount.first);
            return pathUUIDAndAmount.first;
        }
    }
    dropNodeReservationsOnPath(
        reservation.first);
    return UINT64_MAX;
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
    // Ignore self and CoodinatorNode. Coordinator will be asked first
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
        debug() << "receive message with invalid type, ignore it";
        clearContext();
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(2));
    }

    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    const auto kCoordinatorUUID = kMessage->coordinatorUUID();
    const auto kSenderUUID = kMessage->senderUUID;

    if (kCoordinatorUUID == NodeUUID::empty() || kMessage->votes().size() == 0) {
        debug() << "Coordinator don't know result of this transaction yet. Sleep.";
        mVotesRecoveryStep = VotesRecoveryStages::Common_PrepareNodesListToCheckVotes;
        return resultAwaikAfterMilliseconds(
                kWaitMillisecondsToTryRecoverAgain);
    }

    // Check if answer is from Coordinator
    if (kSenderUUID != kCoordinatorUUID){
        debug() << "Sender (" << kSenderUUID << ") is not coordinator ("
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
    error() << "Unexpected behaviour. Apply logic when coordinator didn't sent response";
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
        debug() << "receive message with invalid type, ignore it";
        clearContext();
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(2));
    }

    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    const auto kSenderUUID = kMessage->senderUUID;

    if (kMessage->votes().size() == 0) {
        debug() << "Intermediate node didn't know about this transaction";
        return processNextNodeToCheckVotes();
    }

    if (kSenderUUID != mCurrentNodeToCheckVotes){
        debug() << "Sender is not current checking node";
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
    if (mNodesToCheckVotes.size() == 0) {
        debug() << "No nodes left to be asked. Sleep";
        mVotesRecoveryStep = VotesRecoveryStages::Common_PrepareNodesListToCheckVotes;
        return resultAwaikAfterMilliseconds(
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
    for (const auto nodeUUIDAndReservations : mReservations) {
        for (const auto pathUUIDAndReservation : nodeUUIDAndReservations.second) {
            if (pathUUIDAndReservation.second->direction() == reservationDirection) {
                totalAmount += pathUUIDAndReservation.second->amount();
            }
        }
    }
    return totalAmount;
}

pair<BytesShared, size_t> BasePaymentTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    // mParticipantsVotesMessage Part
    const auto kBufferAndSizeParticipantsVotesMessage = mParticipantsVotesMessage->serializeToBytes();
    // parent part
    size_t bytesCount = parentBytesAndCount.second
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
    const auto kmReservationSize = mReservations.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &kmReservationSize,
        sizeof(uint64_t));
    dataBytesOffset += sizeof(uint64_t);

    for(auto it=mReservations.begin(); it!=mReservations.end(); it++){
        // Map key (NodeUUID)
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &it->first,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;

        // Size of map value vector
        const auto kReservationsValueSize = it->second.size();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &kReservationsValueSize,
            sizeof(uint64_t));
        dataBytesOffset += sizeof(uint64_t);

        for(const auto &kReservationValues: it->second){
            // PathUUID
            memcpy(
                dataBytesShared.get() + dataBytesOffset,
                &kReservationValues.first,
                sizeof(uint64_t));
            dataBytesOffset += sizeof(uint64_t);

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
                sizeof(uint8_t));
            dataBytesOffset += sizeof(uint8_t);
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
                                sizeof(uint64_t) + // PathUUID
                                kTrustLineAmountBytesCount +  // Reservetion Amount
                                TransactionUUID::kBytesSize + // Reservation Transaction UUID
                                sizeof(uint8_t)) * it->second.size() + // Reservation Direction
                                sizeof(uint64_t); // Vector Size

    }
    reservationSizeInBytes += sizeof(uint64_t); // map Size
    return reservationSizeInBytes;
}

const NodeUUID& BasePaymentTransaction::coordinatorUUID() const
{
    return currentNodeUUID();
}

const uint8_t BasePaymentTransaction::cycleLength() const
{
    return 0;
}

bool BasePaymentTransaction::isCommonVotesCheckingstage() const
{
    return mStep == Common_VotesChecking;
}

void BasePaymentTransaction::setRollbackByOtherTransactionStage()
{
    mStep = Common_RollbackByOtherTransaction;
}
