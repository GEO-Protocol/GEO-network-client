#include "ReceiverPaymentTransaction.h"


ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    const NodeUUID &currentNodeUUID,
    ReceiverInitPaymentRequestMessage::ConstShared message,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Logger *log) :

    BasePaymentTransaction(
        BaseTransaction::ReceiverPaymentTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        trustLines,
        storageHandler,
        log),
    mMessage(message),
    mTotalReserved(0)
{
    mStep = Stages::Receiver_CoordinatorRequestApproving;
}

ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Logger *log) :
        BasePaymentTransaction(
            buffer,
            nodeUUID,
            trustLines,
            storageHandler,
            log)
{
    deserializeFromBytes(buffer);
}

TransactionResult::SharedConst ReceiverPaymentTransaction::run()
{
    switch (mStep) {
    case Stages::Receiver_CoordinatorRequestApproving:
        return runInitialisationStage();

    case Stages::Receiver_AmountReservationsProcessing:
        return runAmountReservationStage();

    case Stages::Common_FinalPathsConfigurationChecking:
        return runFinalPathsConfigurationProcessingStage();

    case Stages::Common_VotesChecking:
        return runVotesCheckingStage();

    case Stages::Common_Recovery:
        return runVotesRecoveryParentStage();

    default:
        throw RuntimeError(
            "ReceiverPaymentTransaction::run(): "
            "invalid stage number occurred");
    }
}

pair<BytesShared, size_t> ReceiverPaymentTransaction::serializeToBytes()
{
    throw ValueError("Not implemented");
}

void ReceiverPaymentTransaction::deserializeFromBytes(BytesShared buffer)
{
    throw ValueError("Not implemented");
}

const string ReceiverPaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiverPaymentTA: " << currentTransactionUUID().stringUUID() << "] ";
    return s.str();
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runInitialisationStage()
{
    const auto kCoordinator = mMessage->senderUUID;
    info() << "Operation for " << mMessage->amount() << " initialised by the (" << kCoordinator << ")";


    // Check if total incoming possibilities of the node are <= of the payment amount.
    // If not - there is no reason to process the operation at all.
    // (reject operation)
    const auto kTotalAvailableIncomingAmount = *(mTrustLines->totalIncomingAmount());
    if (kTotalAvailableIncomingAmount < mMessage->amount()) {
        sendMessage<ReceiverInitPaymentResponseMessage>(
            kCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            mMessage->pathUUID(),
            ReceiverInitPaymentResponseMessage::Rejected);

        return exitWithResult(
            resultDone(),
            "Operation rejected due to insufficient funds.");
    }


    sendMessage<ReceiverInitPaymentResponseMessage>(
        kCoordinator,
        currentNodeUUID(),
        currentTransactionUUID(),
        mMessage->pathUUID(),
        ReceiverInitPaymentResponseMessage::Accepted);


    // Begin waiting for amount reservation requests.
    // There is non-zero probability, that first couple of paths will fail.
    // So receiver will wait for time, that is approximatyle neede for several nodes for processing.
    //
    // TODO: enhancement: send aproximate paths count to receiver, so it will be able to wait correct timeout.
    mStep = Stages::Receiver_AmountReservationsProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationRequest},
        maxNetworkDelay(kMaxPathLength * 3));
}

TransactionResult::SharedConst ReceiverPaymentTransaction::runAmountReservationStage()
{
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationRequest))
        return reject("No amount reservation request was received. Rolled back.");


    const auto kMessage = popNextMessage<IntermediateNodeReservationRequestMessage>();
    const auto kNeighbor = kMessage->senderUUID;

    if (! mTrustLines->isNeighbor(kNeighbor)) {
        // Message was sent from node, that is not listed in neighbors list.
        //
        // TODO: enhance this check
        // Neighbor public key must be used here.

        // Ignore received message.
        // Begin waiting for another one.
        //
        // TODO: enhancement: send aproximate paths count to receiver, so it will be able to wait correct timeout.
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay(kMaxPathLength * 3));
    }


    info() << "Amount reservation for " << kMessage->amount() << " request received from " << kNeighbor;

    // Note: copy of shared pointer is required.
    const auto kAvailableAmount = mTrustLines->availableIncomingAmount(kNeighbor);
    if (kMessage->amount() > *kAvailableAmount || ! reserveIncomingAmount(kNeighbor, kMessage->amount(), kMessage->pathUUID())) {
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

        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            kMessage->pathUUID(),
            ResponseMessage::Rejected);

        // Begin accepting other reservation messages
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay(kMaxPathLength * 3));

    }

    const auto kTotalTransactionAmount = mMessage->amount();
    mTotalReserved += kMessage->amount();
    if (mTotalReserved > kTotalTransactionAmount){
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            kMessage->pathUUID(),
            ResponseMessage::Rejected);

        return reject(
            "Reserved amount is greater than requested. "
            "It indicates protocol or realisation error. "
            "Rolled back.");
    }


    info() << "Reserved locally: " << kMessage->amount();
    sendMessage<IntermediateNodeReservationResponseMessage>(
        kNeighbor,
        currentNodeUUID(),
        currentTransactionUUID(),
        kMessage->pathUUID(),
        ResponseMessage::Accepted,
        kMessage->amount());


    if (mTotalReserved == mMessage->amount()) {
        // Reserved amount is enough to move to votes processing stage.

        // TODO: receiver must know, how many paths are involved into the transaction.
        // This info helps to calculate max timeout,
        // that would be used for waiting for votes list message.

        mStep = Stages::Common_VotesChecking;
        return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(kMaxPathLength * 3));

    } else {
        // Waiting for another reservation request
        return resultWaitForMessageTypes(
            {Message::Payments_IntermediateNodeReservationRequest},
            maxNetworkDelay(kMaxPathLength * 3));
    }
}