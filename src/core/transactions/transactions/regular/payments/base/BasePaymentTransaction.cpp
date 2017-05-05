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
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr)
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
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr)
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
    info() << "runVotesCheckingStage";
    // Votes message may be received twice:
    // First time - as a request to check the transaction and to sing it in case if all correct.
    // Second time - as a command to commit/rollback the transaction.
    debug() << "\n\n" << mTransactionIsVoted;

    if (mTransactionIsVoted)
        return runVotesConsistencyCheckingStage();


    if (! contextIsValid(Message::Payments_ParticipantsVotes))
        return reject("No participants votes received. Canceling.");


    const auto kCurrentNodeUUID = currentNodeUUID();
    auto message = popNextMessage<ParticipantsVotesMessage>();
    info() << "Votes message received";


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

        info() << "Votes message ignored due to transactions UUIDs collision detected.";
        info() << "Waiting for another votes message.";

        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(message->participantsCount())); // ToDo: kMessage->participantsCount() must not be used (it is invalid)
    }


    if (message->containsRejectVote())
        // Some node rejected the transaction.
        // This node must simply roll back it's part of transaction and exit.
        // No further message propagation is needed.
        reject("Some participant node has been rejected the transaction. Rolling back.");

    // TODO : insert propagate message here
    message->approve(kCurrentNodeUUID);
    mTransactionIsVoted = true;

    // TODO: flush

    info() << "Voted +";

    try {
        // Try to get next participant from the message.
        // In case if this node is the last node in votes list -
        // then it must be propagated to all nodes as successfully signed transaction.
        const auto kNextParticipant = message->nextParticipant(kCurrentNodeUUID);

        // NotFoundError wasn't thrown.
        // Current node is not last in the votes list.
        // Message must be transferred to the next node in the list.
        sendMessage(
            kNextParticipant,
            message);

        info() << "Votes list message transferred to the (" << kNextParticipant << ")";

        mStep = Stages::Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(message->participantsCount()));

    } catch (NotFoundError &) {
        // There are no nodes left in the votes list.
        // Current node is the last node that has signed the transaction.
        // Now it must be propagated to all nodes in the votes list
        // as successfully signed transaction.
        propagateVotesMessageToAllParticipants(message);

        // Transaction may be committed (all votes are collected and checked).
        return approve();
    }

}

TransactionResult::SharedConst BasePaymentTransaction::runFinalPathConfigurationProcessingStage ()
{
    info() << "runFinalPathConfigurationProcessingStage";
    if (! contextIsValid(Message::Payments_FinalPathConfiguration))
        // Transaction can't be voted so far.
        // As a result - it may be simply cancelled;
        return reject("No final paths configuration was received from the coordinator. Rejected.");


    const auto kMessage = popNextMessage<FinalPathConfigurationMessage>();

    // TODO: check if message was really received from the coordinator;


    info() << "Final payment path configuration received";

    // path was cancelled, drop all reservations belong it
    if (kMessage->amount() == 0) {
        rollBack(kMessage->pathUUID());
    } else {

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
    // TODO correct delay time
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes,
        Message::Payments_IntermediateNodeReservationRequest},
        maxNetworkDelay(kMaxPathLength));
}

/*
 * Handles votes list message receiving or it's absence in case,
 * if current node already voted for the operation.
 * In this case transaction can't be simply cancelled,
 * it must be recovered through recover mechanism to keep data integrity.
 */
TransactionResult::SharedConst BasePaymentTransaction::runVotesConsistencyCheckingStage()
{
    info() << "runVotesConsistencyCheckingStage";
    if (! contextIsValid(Message::Payments_ParticipantsVotes))
        // In case if no votes are present - transaction can't be simply cancelled.
        // It must go through recovery stage to avoid inconsistency.
        return recover("No participants votes received.");


    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    info () << "Participants votes message received.";

    // Checking if no one node has been deleted current nodes sign.
    // (Message modification protection)
    // ToDo: [mvp+] add cryptographic mechanism to prevent removing of votes.
    if (currentNodeUUID() != kMessage->coordinatorUUID())
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

    } else {
        // Otherwise - message contains some uncertain votes.
        // In this case - message may be ignored.
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(kMessage->participantsCount()));
        }
}

const bool BasePaymentTransaction::reserveOutgoingAmount(
    const NodeUUID& neighborNode,
    const TrustLineAmount& amount,
    const PathUUID &pathUUID)
{
    try {
        const auto kReservation = mTrustLines->reserveAmount(
            neighborNode,
            currentTransactionUUID(),
            amount);

#ifdef DEBUG
        debug() << "Reserved " << amount << " for (" << neighborNode << ") [" << pathUUID << "] [Outgoing amount reservation].";
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
        debug() << "Reserved " << amount << " for (" << neighborNode << ") [" << pathUUID << "] [Incoming amount reservation].";
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
    if (mContext.empty())
        return false;

    if (mContext.size() > 1 || mContext.at(0)->typeID() != messageType) {
        if (showErrorMessage) {
            error() << "Unexpected message received. "
                "It seems that remote node doesn't follows the protocol. "
                "Canceling.";
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
        mParticipantsVotesMessage->reject(currentNodeUUID());
        propagateVotesMessageToAllParticipants(mParticipantsVotesMessage);
    }

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
    info() << "Transaction approved. Committing.";

    commit();
    return resultDone();
}

void BasePaymentTransaction::commit ()
{
    info() << "Transaction committing...";

    // TODO: Ensure atomicity in case if some reservations would be used, and transaction crash.

    for (const auto &kNodeUUIDAndReservations : mReservations)
        for (const auto &kPathUUIDAndReservation : kNodeUUIDAndReservations.second) {
            mTrustLines->useReservation(kNodeUUIDAndReservations.first, kPathUUIDAndReservation.second);

            if (kPathUUIDAndReservation.second->direction() == AmountReservation::Outgoing)
                info() << "Committed reservation: [ => ] " << kPathUUIDAndReservation.second->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathUUIDAndReservation.first << "]";

            else if (kPathUUIDAndReservation.second->direction() == AmountReservation::Incoming)
                info() << "Committed reservation: [ <= ] " << kPathUUIDAndReservation.second->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathUUIDAndReservation.first << "]";
            mTrustLines->dropAmountReservation(kNodeUUIDAndReservations.first, kPathUUIDAndReservation.second);
        }

    info() << "Transaction committed.";
}

void BasePaymentTransaction::rollBack ()
{
    for (const auto &kNodeUUIDAndReservations : mReservations)
        for (const auto &kPathUUIDAndReservation : kNodeUUIDAndReservations.second) {
            mTrustLines->dropAmountReservation(kNodeUUIDAndReservations.first, kPathUUIDAndReservation.second);

            if (kPathUUIDAndReservation.second->direction() == AmountReservation::Outgoing)
                info() << "Dropping reservation: [ => ] " << kPathUUIDAndReservation.second->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathUUIDAndReservation.first << "]";

            else if (kPathUUIDAndReservation.second->direction() == AmountReservation::Incoming)
                info() << "Dropping reservation: [ <= ] " << kPathUUIDAndReservation.second->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathUUIDAndReservation.first << "]";
        }
}

void BasePaymentTransaction::rollBack (
    const PathUUID &pathUUID)
{
    auto itNodeUUIDAndReservations = mReservations.begin();
    while(itNodeUUIDAndReservations != mReservations.end()) {
        auto itPathUUIDAndReservation = itNodeUUIDAndReservations->second.begin();
        while (itPathUUIDAndReservation != itNodeUUIDAndReservations->second.end()) {
            if (itPathUUIDAndReservation->first == pathUUID) {
                mTrustLines->dropAmountReservation(
                    itNodeUUIDAndReservations->first,
                    itPathUUIDAndReservation->second);

                if (itPathUUIDAndReservation->second->direction() == AmountReservation::Outgoing)
                    info() << "Dropping reservation: [ => ] " << itPathUUIDAndReservation->second->amount()
                           << " for (" << itNodeUUIDAndReservations->first << ") [" << itPathUUIDAndReservation->first
                           << "]";

                else if (itPathUUIDAndReservation->second->direction() == AmountReservation::Incoming)
                    info() << "Dropping reservation: [ <= ] " << itPathUUIDAndReservation->second->amount()
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
    if (message != nullptr)
        info() << message;

    // TODO: implement me;
    return resultDone();
}

uint32_t BasePaymentTransaction::maxNetworkDelay (
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
    return maxNetworkDelay(1);
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
        const auto kCurrentNodeVote = kMessage->vote(currentNodeUUID());
        if (kCurrentNodeVote == ParticipantsVotesMessage::Approved)
            return true;

    } catch (NotFoundError &){}

    return false;
}

void BasePaymentTransaction::propagateVotesMessageToAllParticipants (
    const ParticipantsVotesMessage::Shared kMessage) const
{
    info() << "propagateVotesMessageToAllParticipants";
    const auto kCurrentNodeUUID = currentNodeUUID();

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

const bool BasePaymentTransaction::shortageReservation (
    const NodeUUID kContractor,
    const AmountReservation::ConstShared kReservation,
    const TrustLineAmount &kNewAmount,
    const PathUUID &pathUUID)
{
    if (kNewAmount > kReservation->amount())
        throw ValueError(
            "BasePaymentTransaction::shortageReservation: "
                "new amount can't be greater than already reserved one.");

    try {
#ifdef DEBUG
        const auto kPreviousAmount = kReservation->amount();
#endif

        auto updatedReservation = mTrustLines->updateAmountReservation(
            kContractor,
            kReservation,
            kNewAmount);

        for (auto it = mReservations[kContractor].begin(); it != mReservations[kContractor].end(); it++){
            // TODO detaild check this condition
            if ((*it).second.get() == kReservation.get() && (*it).first == pathUUID) {
                mReservations[kContractor].erase(it);
                break;
            }
        }
        mReservations[kContractor].push_back(
            make_pair(
                pathUUID,
                updatedReservation));

#ifdef DEBUG
        if (kReservation->direction() == AmountReservation::Incoming)
            info() << "Reservation for (" << kContractor << ") [" << pathUUID << "] shortened "
                   << "from " << kPreviousAmount << " to " << kNewAmount << " [<=]";
        else
            info() << "Reservation for (" << kContractor << ") [" << pathUUID << "] shortened "
                   << "from " << kPreviousAmount << " to " << kNewAmount << " [=>]";
#endif

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

void BasePaymentTransaction::dropReservationsOnPath(
    PathStats *pathStats,
    PathUUID pathUUID)
{
    info() << "dropReservationsOnPath";
    pathStats->shortageMaxFlow(0);

    auto firstIntermediateNode = pathStats->path()->nodes[1];
    // TODO add checking if not find
    auto nodeReservations = mReservations.find(firstIntermediateNode);
    auto itPathUUIDAndReservation = nodeReservations->second.begin();
    while (itPathUUIDAndReservation != nodeReservations->second.end()) {
        if (itPathUUIDAndReservation->first == pathUUID) {
            info() << "Dropping reservation: [ => ] " << itPathUUIDAndReservation->second->amount()
                   << " for (" << firstIntermediateNode << ") [" << pathUUID << "]";
            mTrustLines->dropAmountReservation(
                firstIntermediateNode,
                itPathUUIDAndReservation->second);
            itPathUUIDAndReservation = nodeReservations->second.erase(itPathUUIDAndReservation);
        } else {
            itPathUUIDAndReservation++;
        }
    }
    if (nodeReservations->second.size() == 0) {
        mReservations.erase(firstIntermediateNode);
    }

    const auto lastProcessedNodeAndPos = pathStats->currentIntermediateNodeAndPos();
    const auto lastProcessedNode = lastProcessedNodeAndPos.first;
    info() << "current node " << lastProcessedNode;
    for (const auto &intermediateNode : pathStats->path()->intermediateUUIDs()) {
        if (intermediateNode == lastProcessedNode) {
            break;
        }
        info() << "send message with drop reservation info for node " << intermediateNode;
        sendMessage<FinalPathConfigurationMessage>(
            intermediateNode,
            currentNodeUUID(),
            currentTransactionUUID(),
            pathUUID,
            0);
    }
}

void BasePaymentTransaction::sendFinalPathConfiguration(
    PathStats* pathStats,
    PathUUID pathUUID,
    const TrustLineAmount &finalPathAmount)
{
    info() << "sendFinalPathConfiguration";
    for (const auto &intermediateNode : pathStats->path()->intermediateUUIDs()) {
        info() << "send message with final path amount info for node " << intermediateNode;
        sendMessage<FinalPathConfigurationMessage>(
            intermediateNode,
            currentNodeUUID(),
            currentTransactionUUID(),
            pathUUID,
            finalPathAmount);
    }
}

void BasePaymentTransaction::printReservations() {
    info() << "print reservations";
    for(const auto nodeUUIDAndReservations : mReservations) {
        for (const auto pathUUIDAndReservation : nodeUUIDAndReservations.second) {
            info() << "{" << nodeUUIDAndReservations.first << "} [" << pathUUIDAndReservation.first << "] " << pathUUIDAndReservation.second->amount();
        }
    }
}