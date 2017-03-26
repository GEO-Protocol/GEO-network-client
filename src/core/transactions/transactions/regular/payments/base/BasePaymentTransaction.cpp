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
    if (mTransactionIsVoted)
        return runVotesConsistencyCheckingStage();


    if (! contextIsValid(Message::Payments_ParticipantsVotes))
        return reject("No participants votes received. Canceling.");


    const auto kCurrentNodeUUID = nodeUUID();
    auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    if (kMessage->containsRejectVote())
        // Some node rejected the transaction.
        // This node must simply roll back it's part of transaction and exit.
        // No further message propagation is needed.
        reject("Some participant node has been rejected the transaction. Rolling back.");


    // ToDo: check if node may sign the message
    // (is previous nodes in the paths signed the transaction, etc)

    // TODO: check behaviour when message wasn't approved


    try {
        kMessage->approve(kCurrentNodeUUID);
        mTransactionIsVoted = true;
        info() << "Votes list received. Voted +";

    } catch (NotFoundError &) {
        // It seems that current node wasn't listed in votes list.
        // It is possible only in case when one node takes part in 2 parallel transactions
        // that have common UUID (transactions UUID collision).
        // The probability of this case is very small, but is present.
        //
        // In this case - the message must be simply ignored.

        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxTimeout(kMessage->participantsCount()));
    }

    try {
        // Try to get next participant from the message.
        // In case if this node is the last node in votes list -
        // then it must be propagated to all nodes as successfully signed transaction.
        const auto kNextParticipant = kMessage->nextParticipant(kCurrentNodeUUID);

        // NotFoundError wasn't thrown.
        // Current node is not last in the votes list.
        // Message must be transferred to the next node in the list.
        sendMessage(
            kNextParticipant,
            kMessage);

        info() << "Votes list message transferred to the (" << kNextParticipant << ")";

        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxTimeout(kMessage->participantsCount()));

    } catch (NotFoundError &) {
        // There are no nodes left in the votes list.
        // Current node is the last node that has signed the transaction.
        // Now it must be propagated to all nodes in the votes list
        // as successfully signed transaction.

        auto participant = kMessage->firstParticipant();
        sendMessage(
            participant,
            kMessage);

        for (;;) {
            try {
                participant = kMessage->nextParticipant(participant);

                // Prevent sending packet itself
                if (participant == kCurrentNodeUUID)
                    continue;

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


        // Committing transaction.
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
    if (kMessage->containsRejectVote())
        return reject(
            "Some participant node has been rejected the transaction. Rolling back.");


    if (kMessage->achievedConsensus()){
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
    return
        + (totalParticipantsCount * kMaxMessageTransferLagMSec)
        + (totalParticipantsCount * kExpectedNodeProcessingDelay);
}
