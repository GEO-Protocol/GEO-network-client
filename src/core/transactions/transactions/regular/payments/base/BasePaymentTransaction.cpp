#include "BasePaymentTransaction.h"


BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const NodeUUID &currentNodeUUID,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        type,
        currentNodeUUID,
        log),
    mTrustLines(trustLines),
    mTransactionIsVoted(false)
{}

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &currentNodeUUID,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        type,
        transactionUUID,
        currentNodeUUID,
        log),
    mTrustLines(trustLines),
    mTransactionIsVoted(false)
{}

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        type,
        log),
    mTrustLines(trustLines)
{}

/*
 * Handles votes list message receiving or it's absence.
 * Controls transaction approving/rejecting/rolling back.
 */
TransactionResult::SharedConst BasePaymentTransaction::runVotesCheckingStage()
{
    // Votes message may be received twice:
    // First time - as a request to check the transaction and to sing it in case if all correct.
    // Second time - as a command to commit/rollback the transaction.
    if (mTransactionIsVoted)
        return runVotesConsistencyCheckingStage();


    if (! contextIsValid(Message::Payments_ParticipantsVotes))
        return reject("No participants votes received. Canceling.");


    const auto kCurrentNodeUUID = nodeUUID();
    auto message = popNextMessage<ParticipantsVotesMessage>();


    try {
        // Check if current node is listed in the votes list.
        // This check is needed to prevent processing message in case of missdelivering.
        message->vote(kCurrentNodeUUID);

    } catch (NotFoundError &) {
        // It seems that current node wasn't listed in the votes list.
        // This is possible only in case, when one node takes part in 2 parallel transactions,
        // that have common UUID (transactions UUIDs collision).
        // The probability of this is very small, but is present.
        //
        // In this case - the message must be simply ignored.

        // ToDo: kMessage->participantsCount() must not bu used
        // (this is missdelivered message from another transaction)
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxTimeout(message->participantsCount()));
    }


    if (message->containsRejectVote())
        // Some node rejected the transaction.
        // This node must simply roll back it's part of transaction and exit.
        // No further message propagation is needed.
        reject("Some participant node has been rejected the transaction. Rolling back.");


    // It is possible, that next neighbor node has been reserved less amount,
    // than previous one neighbor node.
    //
    // Asking  the coordinator for the final paths configuration prevents
    // using different amount on incoming and outgoing paths.
    // It is the same step as amount shortage for intermediate nodes.
    sendMessage<ParticipantsConfigurationRequestMessage>(
        message->coordinatorUUID(),
        kCurrentNodeUUID,
        UUID());


    // Votes message must be saved for further processing on next awakening.
    mParticipantsVotesMessage = message;


    mStep = Stages::Common_FinalPathsConfigurationChecking;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsPathsConfiguration},
        maxCoordinatorResponseTimeout());
}

TransactionResult::SharedConst BasePaymentTransaction::runFinalPathsConfigurationProcessingStage ()
{
    if (! contextIsValid(Message::Payments_ParticipantsPathsConfiguration))
        // Transaction can't be voted so far.
        // As a result - it may be simply cancelled;
        return reject("No final paths configuration was received from the coordinator. Rejected.");


    const auto kCurrentNodeUUID = nodeUUID();
    const auto kMessage = popNextMessage<ParticipantsConfigurationMessage>();


    // ToDo: check if node may sign the message
    // (is previous nodes in the paths signed the transaction, etc)

    // TODO: check behaviour when message wasn't approved


    // Shortening of all reservations, that was done by this node
    // Note: copy of reservations is needed to be able to not modify the same reservation twice.
    auto reservationsCopy = mReservations;
    for (const auto kNodesAndFinalAmount : kMessage->nodesAndFinalReservationAmount()) {
        const auto kCommonAmount = *kNodesAndFinalAmount.second;

        for (const auto kNode : kNodesAndFinalAmount.first) {
            auto& nodeReservations = reservationsCopy[kNode];

            while (! nodeReservations.empty()) {
                const auto kMinReservationIterator = min_element(
                    nodeReservations.cbegin(),
                    nodeReservations.cend());

                mTrustLines->updateAmountReservation(
                    kNode,
                    *kMinReservationIterator,
                    kCommonAmount);

                // Prevent updating the same reservation twice
                nodeReservations.erase(kMinReservationIterator);
            }
        }
    }


    mParticipantsVotesMessage->approve(kCurrentNodeUUID);
    mTransactionIsVoted = true;
    info() << "Voted +";

    try {
        // Try to get next participant from the message.
        // In case if this node is the last node in votes list -
        // then it must be propagated to all nodes as successfully signed transaction.
        const auto kNextParticipant = mParticipantsVotesMessage->nextParticipant(kCurrentNodeUUID);

        // NotFoundError wasn't thrown.
        // Current node is not last in the votes list.
        // Message must be transferred to the next node in the list.
        sendMessage(
            kNextParticipant,
            mParticipantsVotesMessage);

        info() << "Votes list message transferred to the (" << kNextParticipant << ")";

        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxTimeout(mParticipantsVotesMessage->participantsCount()));

    } catch (NotFoundError &) {
        // There are no nodes left in the votes list.
        // Current node is the last node that has signed the transaction.
        // Now it must be propagated to all nodes in the votes list
        // as successfully signed transaction.
        propagateVotesMessageToAllParticipants(mParticipantsVotesMessage);

        // Transaction may be committed (all votes are collected and checked).
        return approve();
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
    if (! contextIsValid(Message::Payments_ParticipantsVotes))
        // In case if no votes are present - transaction can't be simply cancelled.
        // It must go through recovery stage to avoid inconsistency.
        return recover();


    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();


    // Checking if no one node has been deleted current nodes sign.
    // (Message modification protection)
    // ToDo: [mvp+] add cryptographic mechanism to prevent removing of votes.
    if (!positiveVoteIsPresent(kMessage)){
        // Note: there is no correct way to exit from the transaction
        // in case if some one removed the vote from the message.
        // Rolling transaction back only reverts current node.

        rollBack();
        throw RuntimeError(
            "BasePaymentTransaction::runVotesConsistencyCheckingStage: "
            "Some node has been modified the message and removed the vote of the current node.");
    }


    if (kMessage->containsRejectVote())
        return reject(
            "Some participant node has been rejected the transaction. Rolling back.");


    if (kMessage->achievedConsensus()){
        // In case if votes message received again -

        info() << "Votes list received. Consensus achieved.";
        return approve();
    }


    // Otherwise - message contains some uncertain votes.
    // In this case - message may be ignored.
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxTimeout(kMessage->participantsCount()));
}

const bool BasePaymentTransaction::reserveOutgoingAmount(
    const NodeUUID& neighborNode,
    const TrustLineAmount& amount)
{
    try {
        const auto kReservation = mTrustLines->reserveAmount(
            neighborNode,
            UUID(),
            amount);

#ifdef DEBUG
        debug() << "Reserved outgoing " << amount << " for (" << neighborNode << ").";
#endif

        mReservations[neighborNode].push_back(kReservation);
        return true;

    } catch (Exception &) {}

    return false;
}

const bool BasePaymentTransaction::reserveIncomingAmount(
    const NodeUUID& neighborNode,
    const TrustLineAmount& amount)
{
    try {
        const auto kReservation = mTrustLines->reserveIncomingAmount(
            neighborNode,
            UUID(),
            amount);

#ifdef DEBUG
        debug() << "Reserved incoming " << amount << " for (" << neighborNode << ").";
#endif

        mReservations[neighborNode].push_back(kReservation);
        return true;

    } catch (Exception &) {}

    return false;
}

const bool BasePaymentTransaction::contextIsValid(
    Message::MessageTypeID messageType) const
{
    if (mContext.empty())
        return false;

    if (mContext.size() > 1 || mContext.at(0)->typeID() != messageType) {
        error() << "Unexpected message received. "
                   "It seems that remote node doesn't follows the protocol. "
                   "Canceling.";

        return false;
    }

    return true;
}

/*
 * Shortcut method for rejecting transaction.
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

    mParticipantsVotesMessage->reject(nodeUUID());
    propagateVotesMessageToAllParticipants(mParticipantsVotesMessage);

    rollBack();
    return exit();
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
    info() << "Transaction approved. Committing.";

    commit();
    return exit();
}

void BasePaymentTransaction::commit ()
{
    info() << "Transaction committing...";

    // TODO: Ensure atomicity in case if some reservations would be used, and transaction crash.

    for (const auto &kNodeUUIDAndReservations : mReservations)
        for (const auto &kReservation : kNodeUUIDAndReservations.second) {
            mTrustLines->useReservation(kNodeUUIDAndReservations.first, kReservation);

            if (kReservation->direction() == AmountReservation::Outgoing)
                info() << "Committed reservation: [ => ] " << kReservation->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ")";

            else if (kReservation->direction() == AmountReservation::Incoming)
                info() << "Committed reservation: [ <= ] " << kReservation->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ")";
        }

    info() << "Transaction committed.";
}

void BasePaymentTransaction::rollBack ()
{
    info() << "Transaction rolling back...";

    for (const auto &kNodeUUIDAndReservations : mReservations)
        for (const auto &kReservation : kNodeUUIDAndReservations.second) {
            mTrustLines->dropAmountReservation(kNodeUUIDAndReservations.first, kReservation);

            if (kReservation->direction() == AmountReservation::Outgoing)
                info() << "Dropping reservation: [ => ] " << kReservation->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ")";

            else if (kReservation->direction() == AmountReservation::Incoming)
                info() << "Dropping reservation: [ <= ] " << kReservation->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ")";
        }

    info() << "Transaction rolled back.";
}

TransactionResult::SharedConst BasePaymentTransaction::recover ()
{
    info() << "Recover";
    return exit();
}

uint32_t BasePaymentTransaction::maxTimeout (
    const uint16_t totalParticipantsCount) const
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(totalParticipantsCount > 0);
#endif

    return
        + (totalParticipantsCount * kMaxMessageTransferLagMSec * 2) // double message trip
        + (totalParticipantsCount * kExpectedNodeProcessingDelay);
}

uint32_t BasePaymentTransaction::maxCoordinatorResponseTimeout () const
{
    return maxTimeout(1);
}

/**
 * @returns true in case if "kMessage" contains positive fote for the transaction.
 * Otherwise - returns false.
 */
const bool BasePaymentTransaction::positiveVoteIsPresent (
    const ParticipantsVotesMessage::ConstShared kMessage) const
{
    // ToDo: add cryptographic mechanism to prevent removing of votes.

    try {
        const auto kCurrentNodeVote = kMessage->vote(nodeUUID());
        if (kCurrentNodeVote == ParticipantsVotesMessage::Approved)
            return true;

    } catch (NotFoundError &){}

    return false;
}

void BasePaymentTransaction::propagateVotesMessageToAllParticipants (
    const ParticipantsVotesMessage::Shared kMessage) const
{
    const auto kCurrentNodeUUID = nodeUUID();

    auto participant = kMessage->firstParticipant();
    if (participant != kCurrentNodeUUID)
        sendMessage(
            participant,
            kMessage);

    for (;;) {
        try {
            participant = kMessage->nextParticipant(participant);
            if (participant != kCurrentNodeUUID)
                sendMessage(
                    participant,
                    kMessage);

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
