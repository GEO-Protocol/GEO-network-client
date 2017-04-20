#include "IntermediateNodePaymentTransaction.h"


IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    const NodeUUID& currentNodeUUID,
    IntermediateNodeReservationRequestMessage::ConstShared message,
    TrustLinesManager* trustLines,
    PaymentOperationStateHandler *paymentOperationStateHandler,
    Logger* log) :

    BasePaymentTransaction(
        BaseTransaction::IntermediateNodePaymentTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        trustLines,
        paymentOperationStateHandler,
        log),
    mMessage(message)
{
    mStep = Stages::IntermediateNode_PreviousNeighborRequestProcessing;
}

IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    BytesShared buffer,
    TrustLinesManager* trustLines,
    PaymentOperationStateHandler *paymentOperationState,
    Logger* log) :

        BasePaymentTransaction(
                BaseTransaction::IntermediateNodePaymentTransaction,
                buffer,
                trustLines,
                paymentOperationState,
                log)
{}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::run()
{
    switch (mStep) {
    case Stages::IntermediateNode_PreviousNeighborRequestProcessing:
        return runPreviousNeighborRequestProcessingStage();

    case Stages::IntermediateNode_CoordinatorRequestProcessing:
        return runCoordinatorRequestProcessingStage();

    case Stages::IntermediateNode_NextNeighborResponseProcessing:
        return runNextNeighborResponseProcessingStage();

    case Stages::IntermediateNode_ReservationProlongation:
        return runReservationProlongationStage();

    case Stages::Common_FinalPathsConfigurationChecking:
        return runFinalPathsConfigurationProcessingStage();

    case Stages::Common_VotesChecking:
        return runVotesCheckingStage();

    default:
        throw RuntimeError(
            "IntermediateNodePaymentTransaction::run: "
            "unexpected stage occurred.");
    }
}

pair<BytesShared, size_t> IntermediateNodePaymentTransaction::serializeToBytes()
{
    throw Exception("Not implemented");
}


TransactionResult::SharedConst IntermediateNodePaymentTransaction::runPreviousNeighborRequestProcessingStage()
{
    const auto kNeighbor = mMessage->senderUUID;
    info() << "Init. intermediate payment operation from node (" << kNeighbor << ")";
    info() << "Requested amount reservation: " << mMessage->amount();


    // Note: (copy of shared pointer is required)
    const auto kIncomingAmount = mTrustLines->availableIncomingAmount(kNeighbor);
    const auto kReservationAmount =
        min(mMessage->amount(), *kIncomingAmount);

    if (0 == kReservationAmount || ! reserveIncomingAmount(kNeighbor, kReservationAmount)) {
        sendMessage<IntermediateNodeReservationResponseMessage>(
            kNeighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseMessage::Rejected);
        return reject("No incoming amount reservation is possible. Rolled back.");
    }


    mLastReservedAmount = kReservationAmount;
    sendMessage<IntermediateNodeReservationResponseMessage>(
        kNeighbor,
        currentNodeUUID(),
        currentTransactionUUID(),
        ResponseMessage::Accepted);


    mStep = Stages::IntermediateNode_CoordinatorRequestProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_CoordinatorReservationRequest},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runCoordinatorRequestProcessingStage()
{
    if (! contextIsValid(Message::Payments_CoordinatorReservationRequest))
        return reject("No coordinator request received. Rolled back.");


    info() << "Coordinator further reservation request received.";


    // TODO: add check for previous nodes amount reservation


    const auto kMessage = popNextMessage<CoordinatorReservationRequestMessage>();
    const auto kNextNode = kMessage->nextNodeInPathUUID();
    mCoordinator = kMessage->senderUUID;



    // Note: copy of shared pointer is required
    const auto kOutgoingAmount = mTrustLines->availableOutgoingAmount(kNextNode);
    TrustLineAmount reservationAmount = min(
        kMessage->amount(),
        *kOutgoingAmount);

    if (0 == reservationAmount || ! reserveOutgoingAmount(kNextNode, reservationAmount)) {
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseMessage::Rejected);

        return reject("No amount reservation is possible. Rolled back.");
    }


    mLastReservedAmount = reservationAmount;
    sendMessage<IntermediateNodeReservationRequestMessage>(
        kNextNode,
        currentNodeUUID(),
        currentTransactionUUID(),
        reservationAmount);

    clearContext();
    mStep = Stages::IntermediateNode_NextNeighborResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::Payments_IntermediateNodeReservationResponse},
        maxNetworkDelay(2));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runNextNeighborResponseProcessingStage()
{
    if (! contextIsValid(Message::Payments_IntermediateNodeReservationResponse))
        reject("No valid amount reservation response received. Rolled back.");


    const auto kMessage = popNextMessage<IntermediateNodeReservationResponseMessage>();
    const auto kContractor = kMessage->senderUUID;


    if (kMessage->state() == IntermediateNodeReservationResponseMessage::Rejected){
        sendMessage<CoordinatorReservationResponseMessage>(
            mCoordinator,
            currentNodeUUID(),
            currentTransactionUUID(),
            ResponseMessage::Rejected);
        return reject("Amount reservation rejected by the neighbor node. Rolled back.");
    }


    info() << "(" << kContractor << ") accepted amount reservation.";
    sendMessage<CoordinatorReservationResponseMessage>(
        mCoordinator,
        currentNodeUUID(),
        currentTransactionUUID(),
        ResponseMessage::Accepted,
        mLastReservedAmount);


    mStep = Stages::IntermediateNode_ReservationProlongation;
    return resultWaitForMessageTypes(
        {Message::Payments_ParticipantsVotes},
        maxNetworkDelay(kMaxPathLength));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runReservationProlongationStage()
{
    // In case if participants votes message is already received -
    // there is no need to prolong reservation, transaction may be proceeded.
    if (! mContext.empty())
        if (mContext.at(0)->typeID() == Message::Payments_ParticipantsVotes) {
            mStep = Stages::Common_VotesChecking;
            return runVotesCheckingStage();
        }

    return reject("No participants votes message received. Rolling back.");
}

void IntermediateNodePaymentTransaction::deserializeFromBytes(
    BytesShared buffer)
{
    throw Exception("Not implemented");
}

const string IntermediateNodePaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[IntermediateNodePaymentTA: " << currentTransactionUUID().stringUUID() << "] ";

    return s.str();
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::runPreviousNeighborVoutesRequestStage() {

    const auto kNeighbor = mMessage->senderUUID;
    mStep = Stages::Common_CheckPreviousNeighborVoutesRequestStage;
    return sendVoutesRequestMessageAndWaitForResponse(kNeighbor);
}



TransactionResult::SharedConst IntermediateNodePaymentTransaction::runCheckPreviousNeighborVoutesStage() {

    if(mContext.size() != 1) {
        const NodeUUID kNextNeighbor;
        mStep = Stages::Common_CheckCoordinatorNeighborVoutesStage;
        return sendVoutesRequestMessageAndWaitForResponse(kNextNeighbor);
    }
    const auto kMessage = popNextMessage<ParticipantsVotesMessage>();
    if(kMessage->containsRejectVote()){
        // todo add info message
        return resultDone();
    }
    return resultWaitForMessageTypes(
            {Message::Payments_ParticipantsVotes},
            maxNetworkDelay(kMaxPathLength));
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::sendVoutesRequestMessageAndWaitForResponse(
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
