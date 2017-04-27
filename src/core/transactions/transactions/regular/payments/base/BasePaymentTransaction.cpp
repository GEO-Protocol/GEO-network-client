#include "BasePaymentTransaction.h"
#include <chrono>
#include <thread>

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const NodeUUID &currentNodeUUID,
    TrustLinesManager *trustLines,
    PaymentOperationStateHandler *paymentOperationStateHandler,
    Logger *log) :

    BaseTransaction(
        type,
        currentNodeUUID,
        log),
    mTrustLines(trustLines),
    mPaymentOperationState(paymentOperationStateHandler),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr)
{}

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &currentNodeUUID,
    TrustLinesManager *trustLines,
    PaymentOperationStateHandler *paymentOperationStateHandler,
    Logger *log) :

    BaseTransaction(
        type,
        transactionUUID,
        currentNodeUUID,
        log),
    mTrustLines(trustLines),
    mPaymentOperationState(paymentOperationStateHandler),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr)
{}

BasePaymentTransaction::BasePaymentTransaction(
        const TransactionType type,
        BytesShared buffer,
        TrustLinesManager *trustLines,
        PaymentOperationStateHandler *paymentOperationStateHandler,
        Logger *log) :

    BaseTransaction(
        type,
        log),
    mTrustLines(trustLines),
    mPaymentOperationState(paymentOperationStateHandler)
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


    // It is possible, that next neighbor node has been reserved less amount,
    // than previous one neighbor node.
    //
    // Asking  the coordinator for the final paths configuration prevents
    // using different amount on incoming and outgoing paths.
    // It is the same step as amount shortage for intermediate nodes.
    sendMessage<ParticipantsConfigurationRequestMessage>(
        message->coordinatorUUID(),
        kCurrentNodeUUID,
        currentTransactionUUID());
    // todo remove debug code
//    const NodeUUID debugNodeUUID = NodeUUID("c3642755-7b0a-4420-b7b0-2dcf578d88ca");
//    if(mNodeUUID == debugNodeUUID) {
//        cout << "Debug mode. Wait for recovery" << endl;
//        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//        return recover("Debug recover");
//    }
    info() << "Final payment paths configuration requested from the coordinator.";


    // Votes message must be saved for further processing on next awakening.
    mParticipantsVotesMessage = message;


    mStep = Stages::Common_FinalPathsConfigurationChecking;
    // todo add debug code to drop node
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


    const auto kCurrentNodeUUID = currentNodeUUID();
    const auto kMessage = popNextMessage<ParticipantsConfigurationMessage>();

    // TODO: check if message was really received from the coordinator;


    info() << "Final payment paths configuration received";

    // ToDo: check if node may sign the message
    // (is previous nodes in the paths signed the transaction, etc)

    // TODO: check behaviour when message wasn't approved


    // Shortening all reservations that belongs to this node.
    //
    // Note: reservations copy is needed to be able to remove records from the reservations map.
    // This approach significantly simplifies the logic
    // and gives ability to not update the same reservation twice.
    auto localReservationsCopy = mReservations;
    for (const auto kNodesAndFinalAmount : kMessage->nodesAndFinalReservationAmount()) {
        const auto kCommonPathAmount = *kNodesAndFinalAmount.second;

        for (const auto kNode : kNodesAndFinalAmount.first) {
            auto& nodeReservations = localReservationsCopy[kNode];

            while (! nodeReservations.empty()) {
                const auto kMinReservationIterator = min_element(
                    nodeReservations.cbegin(),
                    nodeReservations.cend());

                const auto kMinReservation = *kMinReservationIterator;
                shortageReservation(
                    kNode,
                    kMinReservation,
                    kCommonPathAmount);

                // Prevent updating the same reservation twice
                nodeReservations.erase(kMinReservationIterator);
            }
        }
    }


    mParticipantsVotesMessage->approve(kCurrentNodeUUID);
    mTransactionIsVoted = true;

    // TODO: flush

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
        // debug code
//        const NodeUUID debugNodeUUID = NodeUUID("c3642755-7b0a-4420-b7b0-2dcf578d88ca");
//        if(mNodeUUID == debugNodeUUID) {
//            cout << "Debug mode. Wait for recovery" << endl;
//            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//            return recover("Debug recover");
//        }
        // debug code
        mStep = Stages::Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(mParticipantsVotesMessage->participantsCount()));

    } catch (NotFoundError &) {
        // There are no nodes left in the votes list.
        // Current node is the last node that has signed the transaction.
        // Now it must be propagated to all nodes in the votes list
        // as successfully signed transaction.
        propagateVotesMessageToAllParticipants(mParticipantsVotesMessage);
        // debug code
        cout << "propagateVotesMessageToAllParticipants(mParticipantsVotesMessage);" << endl;
        //debug code
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
    const TrustLineAmount& amount)
{
    try {
        const auto kReservation = mTrustLines->reserveAmount(
            neighborNode,
            currentTransactionUUID(),
            amount);

#ifdef DEBUG
        debug() << "Reserved " << amount << " for (" << neighborNode << ") [Outgoing amount reservation].";
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
            currentTransactionUUID(),
            amount);

#ifdef DEBUG
        debug() << "Reserved " << amount << " for (" << neighborNode << ") [Incoming amount reservation].";
#endif

        mReservations[neighborNode].push_back(kReservation);
        return true;

    } catch (Exception &) {}

    return false;
}

const bool BasePaymentTransaction::contextIsValid(
    Message::MessageType messageType) const
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
        for (const auto &kReservation : kNodeUUIDAndReservations.second) {
            mTrustLines->useReservation(kNodeUUIDAndReservations.first, kReservation);

            if (kReservation->direction() == AmountReservation::Outgoing)
                info() << "Committed reservation: [ => ] " << kReservation->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ")";

            else if (kReservation->direction() == AmountReservation::Incoming)
                info() << "Committed reservation: [ <= ] " << kReservation->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ")";
        }

    saveVoutes();
    info() << "Voutes saved.";
    info() << "Transaction committed.";
}

void BasePaymentTransaction::saveVoutes()
{
    auto bufferAndSize = mParticipantsVotesMessage->serializeToBytes();
    mPaymentOperationState->saveRecord(
            mParticipantsVotesMessage->transactionUUID(),
            bufferAndSize.first,
            bufferAndSize.second
    );
}

void BasePaymentTransaction::rollBack ()
{
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
}

TransactionResult::SharedConst BasePaymentTransaction::recover (
    const char *message)
{
    if (message != nullptr)
        info() << message;

    mStep = Stages::Common_VotesRecoveryStage;
    mVotesRecoveryStep = VoutesRecoveryStages::Common_PrepareNodesListToCheckVotes;
    return runVotesRecoveryParenStage();
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
    const TrustLineAmount &kNewAmount)
{
    if (kNewAmount > kReservation->amount())
        throw ValueError(
            "BasePaymentTransaction::shortageReservation: "
                "new amount can't be greater than already reserved one.");

    try {
#ifdef DEBUG
        const auto kPreviousAmount = kReservation->amount();
#endif

        mTrustLines->updateAmountReservation(
            kContractor,
            kReservation,
            kNewAmount);

#ifdef DEBUG
        if (kReservation->direction() == AmountReservation::Incoming)
            info() << "Reservation for (" << kContractor << ") shortened "
                   << "from " << kPreviousAmount << " to " << kNewAmount << " [=>]";
        else
            info() << "Reservation for (" << kContractor << ") shortened "
                   << "from " << kPreviousAmount << " to " << kNewAmount << " [<=]";
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

TransactionResult::SharedConst BasePaymentTransaction::runVotesRecoveryParenStage() {
    cout << "BasePaymentTransaction::runVotesRecoveryParenStage()" << endl;
    switch (mVotesRecoveryStep) {
        case VoutesRecoveryStages ::Common_PrepareNodesListToCheckVotes:
            return runPrepareListNodesToCheckNodes();
        case VoutesRecoveryStages ::Common_CheckCoordinatorVotesStage:
            return runCheckCoordinatorVotesStage();
        case VoutesRecoveryStages ::Common_CheckIntermediateNodeVotesStage:
            return runCheckIntermediateNodeVotesSage();

        default:
            throw RuntimeError(
                "CoordinatorPaymentTransaction::run(): "
                    "invalid transaction step.");
    }
}

TransactionResult::SharedConst BasePaymentTransaction::sendVoutesRequestMessageAndWaitForResponse(
    const NodeUUID &contractorUUID)
{
    auto requestMessage = make_shared<VotesStatusRequestMessage>(
        mNodeUUID,
        currentTransactionUUID()
    );
    sendMessage(
        contractorUUID,
        requestMessage
    );
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxNetworkDelay(kMaxPathLength));
}

TransactionResult::SharedConst BasePaymentTransaction::runPrepareListNodesToCheckNodes() {
    const auto kCoordinatorUUID = mParticipantsVotesMessage->coordinatorUUID();
    for(const auto kNodeUUIDAndVote: mParticipantsVotesMessage->votes()){
        if (kNodeUUIDAndVote.first != kCoordinatorUUID)
            mNodesToCheckVotes.push_back(kNodeUUIDAndVote.first);
    }
    // todo add check on zero MParicipationMessage
    if(kCoordinatorUUID == mNodeUUID) {
        mVotesRecoveryStep = VoutesRecoveryStages::Common_CheckIntermediateNodeVotesStage;
        mCurrentNodeToCheckVotes = mNodesToCheckVotes.back();
        mNodesToCheckVotes.pop_back();
        return sendVoutesRequestMessageAndWaitForResponse(mCurrentNodeToCheckVotes);
    }
    mVotesRecoveryStep = VoutesRecoveryStages::Common_CheckCoordinatorVotesStage;
    return sendVoutesRequestMessageAndWaitForResponse(kCoordinatorUUID);
}

TransactionResult::SharedConst BasePaymentTransaction::runCheckCoordinatorVotesStage() {

    if (mContext.size() == 1) {
        const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
        if (mParticipantsVotesMessage->votes().size() > 0) {
            saveVoutes();
            if (kMessage->containsRejectVote()) {
                mParticipantsVotesMessage = kMessage;
                return reject("");
            } else{
                commit();
                return resultDone();
            }
        }
    }
    // Coordinator Node is offline
    mContext.clear();
    mVotesRecoveryStep = VoutesRecoveryStages::Common_CheckIntermediateNodeVotesStage;
    mCurrentNodeToCheckVotes = mNodesToCheckVotes.back();
    mNodesToCheckVotes.pop_back();
    return sendVoutesRequestMessageAndWaitForResponse(mCurrentNodeToCheckVotes);
}

TransactionResult::SharedConst BasePaymentTransaction::runCheckIntermediateNodeVotesSage() {
    if (mContext.size() == 1) {
        const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
        if (mParticipantsVotesMessage->votes().size() > 0) {
            saveVoutes();
            if (kMessage->containsRejectVote()) {
                mParticipantsVotesMessage = kMessage;
                return reject("");
            } else {
                commit();
                return resultDone();
            }
        }
    }
    // Node that was asked has no information about transaction status
    if (mNodesToCheckVotes.size() > 0){
        // Ask another node from payment transaction
        mCurrentNodeToCheckVotes = mNodesToCheckVotes.back();
        mNodesToCheckVotes.pop_back();
        return sendVoutesRequestMessageAndWaitForResponse(mCurrentNodeToCheckVotes);
    }
    // No nodes left to be asked. reject
    return reject("");
}
