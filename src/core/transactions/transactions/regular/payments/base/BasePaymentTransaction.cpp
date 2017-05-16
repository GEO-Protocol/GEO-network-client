#include "BasePaymentTransaction.h"

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const NodeUUID &currentNodeUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger *log) :

    BaseTransaction(
        type,
        currentNodeUUID,
        log),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
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
    Logger *log) :

    BaseTransaction(
        type,
        transactionUUID,
        currentNodeUUID,
        log),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mTransactionIsVoted(false),
    mParticipantsVotesMessage(nullptr)
{}

BasePaymentTransaction::BasePaymentTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *log) :

    BaseTransaction(
        buffer,
        nodeUUID,
        log),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager)
{}

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
            maxNetworkDelay(mParticipantsVotesMessage->participantsCount())); // ToDo: kMessage->participantsCount() must not be used (it is invalid)
    }


    if (mParticipantsVotesMessage->containsRejectVote())
        // Some node rejected the transaction.
        // This node must simply roll back it's part of transaction and exit.
        // No further message propagation is needed.
        reject("Some participant node has been rejected the transaction. Rolling back.");

    // TODO : insert propagate message here
    mParticipantsVotesMessage->approve(kCurrentNodeUUID);
    mTransactionIsVoted = true;

    // TODO: flush

    debug() << "Voted +";

    try {
        // Try to get next participant from the message.
        // In case if this node is the last node in votes list -
        // then it must be propagated to all nodes as successfully signed transaction.
        const auto kNextParticipant = mParticipantsVotesMessage->nextParticipant(kCurrentNodeUUID);
        const auto kNewParticipantsVotesMessage  = make_shared<ParticipantsVotesMessage>(
            mNodeUUID,
            mParticipantsVotesMessage
        );

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
            maxNetworkDelay(mParticipantsVotesMessage->participantsCount()));

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

TransactionResult::SharedConst BasePaymentTransaction::runFinalPathConfigurationProcessingStage ()
{
    debug() << "runFinalPathConfigurationProcessingStage";
    if (! contextIsValid(Message::Payments_FinalPathConfiguration))
        return reject("No final paths configuration was received from the coordinator. Rejected.");


    const auto kMessage = popNextMessage<FinalPathConfigurationMessage>();

    // TODO: check if message was really received from the coordinator;


    debug() << "Final payment path configuration received";

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
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes,
        Message::Payments_IntermediateNodeReservationRequest},
        maxNetworkDelay(kMaxPathLength - 2));
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
    // this case can be only in Coordinator transaction.
    // Intermediate node or Receiver can send request if transaction is still alive.
    if (contextIsValid(Message::Payments_TTLProlongation, false)) {
        return runTTLTransactionResponce();
    }
    if (! contextIsValid(Message::Payments_ParticipantsVotes))
        // In case if no votes are present - transaction can't be simply cancelled.
        // It must go through recovery stage to avoid inconsistency.
        return recover("No participants votes received.");


    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    debug () << "Participants votes message received.";

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

        debug() << "Votes list received. Consensus achieved.";
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
        saveVotes();
        const auto kNewParticipantsVotesMessage  = make_shared<ParticipantsVotesMessage>(
          mNodeUUID,
          mParticipantsVotesMessage
        );
        propagateVotesMessageToAllParticipants(kNewParticipantsVotesMessage);
    }

    rollBack();
    debug() << "Transaction succesfully rolled back.";

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
    return resultDone();
}

void BasePaymentTransaction::commit ()
{
    debug() << "Transaction committing...";

//    // TODO: Ensure atomicity in case if some reservations would be used, and transaction crash.
//    {
//        const auto ioTransaction = mStorageHandler->beginTransaction();
//    }

    for (const auto &kNodeUUIDAndReservations : mReservations)
        for (const auto &kPathUUIDAndReservation : kNodeUUIDAndReservations.second) {
            mTrustLines->useReservation(kNodeUUIDAndReservations.first, kPathUUIDAndReservation.second);

            if (kPathUUIDAndReservation.second->direction() == AmountReservation::Outgoing)
                debug() << "Committed reservation: [ => ] " << kPathUUIDAndReservation.second->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathUUIDAndReservation.first << "]";

            else if (kPathUUIDAndReservation.second->direction() == AmountReservation::Incoming)
                debug() << "Committed reservation: [ <= ] " << kPathUUIDAndReservation.second->amount()
                       << " for (" << kNodeUUIDAndReservations.first << ") [" << kPathUUIDAndReservation.first << "]";
            mTrustLines->dropAmountReservation(kNodeUUIDAndReservations.first, kPathUUIDAndReservation.second);
        }

    // reset initiator cashe, becouse after changing balanses
    // we need updated information on max flow calculation transaction
    mMaxFlowCalculationCacheManager->resetInititorCache();
    debug() << "Transaction committed.";
    saveVotes();
    info() << "Voutes saved.";
    info() << "Transaction committed.";
    // TODO: Ensure atomicity in case if some reservations would be used, and transaction crash.
    {
        const auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->transactionHandler()->deleteRecord(currentTransactionUUID());
    }
}

void BasePaymentTransaction::saveVotes()
{
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
    for (const auto &kNodeUUIDAndReservations : mReservations)
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

    if(mTransactionIsVoted){
        mStep = Stages::Common_Recovery;
        mVotesRecoveryStep = VotesRecoveryStages::Common_PrepareNodesListToCheckVotes;
        return runVotesRecoveryParentStage();
    } else {
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
    debug() << "propagateVotesMessageToAllParticipants";
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
            debug() << "Reservation for (" << kContractor << ") [" << pathUUID << "] shortened "
                   << "from " << kPreviousAmount << " to " << kNewAmount << " [<=]";
        else
            debug() << "Reservation for (" << kContractor << ") [" << pathUUID << "] shortened "
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

TransactionResult::SharedConst BasePaymentTransaction::runTTLTransactionResponce()
{
    auto kMessage = popNextMessage<TTLPolongationMessage>();
    sendMessage<TTLPolongationMessage>(
        kMessage->senderUUID,
        currentNodeUUID(),
        currentTransactionUUID());
    debug() << "Send clarifying message that transactions is alive to node " << kMessage->senderUUID;
    return resultContinuePreviousState();
}

void BasePaymentTransaction::dropReservationsOnPath(
    PathStats *pathStats,
    PathUUID pathUUID)
{
    debug() << "dropReservationsOnPath";
    pathStats->shortageMaxFlow(0);

    auto firstIntermediateNode = pathStats->path()->nodes[1];
    // TODO add checking if not find
    auto nodeReservations = mReservations.find(firstIntermediateNode);
    auto itPathUUIDAndReservation = nodeReservations->second.begin();
    while (itPathUUIDAndReservation != nodeReservations->second.end()) {
        if (itPathUUIDAndReservation->first == pathUUID) {
            debug() << "Dropping reservation: [ => ] " << itPathUUIDAndReservation->second->amount()
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
    debug() << "current node " << lastProcessedNode;
    for (const auto &intermediateNode : pathStats->path()->intermediateUUIDs()) {
        if (intermediateNode == lastProcessedNode) {
            break;
        }
        debug() << "send message with drop reservation info for node " << intermediateNode;
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


TransactionResult::SharedConst BasePaymentTransaction::runVotesRecoveryParentStage()
{
    debug() << "runVotesRecoveryParentStage";
    switch (mVotesRecoveryStep) {
        case VotesRecoveryStages ::Common_PrepareNodesListToCheckVotes:
            return runPrepareListNodesToCheckNodes();
        case VotesRecoveryStages ::Common_CheckCoordinatorVotesStage:
            return runCheckCoordinatorVotesStage();
        case VotesRecoveryStages ::Common_CheckIntermediateNodeVotesStage:
            return runCheckIntermediateNodeVotesSage();

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
        currentTransactionUUID()
    );
    sendMessage(
        contractorUUID,
        requestMessage
    );
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxNetworkDelay(1));
}

TransactionResult::SharedConst BasePaymentTransaction::runPrepareListNodesToCheckNodes()
{
    debug() << "runPrepareListNodesToCheckNodes";
    // Add all nodes that could be ased for Votes Status.
    //Ignore self and CoodinatorNOde. Coordinator wil be asked first
    const auto kCoordinatorUUID = mParticipantsVotesMessage->coordinatorUUID();
    for(const auto kNodeUUIDAndVote: mParticipantsVotesMessage->votes()){
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
    if (mContext.size() == 1) {
        const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
        const auto kCoordinatorUUID = kMessage->coordinatorUUID();
        const auto kSenderUUID = kMessage->senderUUID;
        // Check if answer is from Coordinator
        if (kSenderUUID != kCoordinatorUUID){
            return resultWaitForMessageTypes(
                {Message::Payments_ParticipantsVotes},
                maxNetworkDelay(1));
        }
        if (mParticipantsVotesMessage->votes().size() > 0) {
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
    mVotesRecoveryStep = VotesRecoveryStages::Common_CheckIntermediateNodeVotesStage;
    mCurrentNodeToCheckVotes = mNodesToCheckVotes.back();
    mNodesToCheckVotes.pop_back();
    return sendVotesRequestMessageAndWaitForResponse(mCurrentNodeToCheckVotes);
}

TransactionResult::SharedConst BasePaymentTransaction::runCheckIntermediateNodeVotesSage()
{
    debug() << "runCheckIntermediateNodeVotesSage";
    if (mContext.size() == 1) {
        const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
        const auto kSenderUUID = kMessage->senderUUID;
        if (kSenderUUID != mCurrentNodeToCheckVotes){
            return resultWaitForMessageTypes(
                {Message::Payments_ParticipantsVotes},
                maxNetworkDelay(1));
        }
        if (mParticipantsVotesMessage->votes().size() > 0) {
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
        return sendVotesRequestMessageAndWaitForResponse(mCurrentNodeToCheckVotes);
    }
    // No nodes left to be asked. reject
    return reject("");
}



pair<BytesShared, size_t> BasePaymentTransaction::serializeToBytes() const {
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    // parent part
//    size_t bytesCount = parentBytesAndCount.second
//                        + neighborsCount * NodeUUID::kBytesSize;
//
//    BytesShared dataBytesShared = tryCalloc(bytesCount);
//    size_t dataBytesOffset = 0;
    return parentBytesAndCount;
}

size_t BasePaymentTransaction::reservationsSizeInBytes() const {
//    size_t reservationSizeInBytes = 0;
//    for (auto it=mReservations.begin(); it!=mReservations.end(); it++){
//        reservationSizeInBytes += NodeUUID::kBytesSize + (
//                                                     uint16_t + // Path
//                                                     kTrustLineAmountBytesCount +  // Reservetion Amount
//                                                     TransactionUUID::kBytesSize + // Reservation Transaction UUID
//                                                     uint8_t) * it->second.size() // Reservation Direction
//            + uint16_t; // Vector Size
//
//    }
    return 0;
}
