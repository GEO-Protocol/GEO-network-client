#include "IntermediateNodePaymentTransaction.h"


IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    const NodeUUID& currentNodeUUID,
    ReserveBalanceRequestMessage::ConstShared message,
    TrustLinesManager* trustLines,
    Logger* log) :

    BasePaymentTransaction(
        BaseTransaction::IntermediateNodePaymentTransaction,
        message->transactionUUID(),
        currentNodeUUID,
        trustLines,
        log),
    mMessage(message)
{}

IntermediateNodePaymentTransaction::IntermediateNodePaymentTransaction(
    BytesShared buffer,
    TrustLinesManager* trustLines,
    Logger* log) :

    BasePaymentTransaction(
        BaseTransaction::IntermediateNodePaymentTransaction,
        buffer,
        trustLines,
        log)
{}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::run()
{
    switch (mStep) {
    case 1:
        return initOperation();

    default:
        throw RuntimeError(
            "IntermediateNodePaymentTransaction::run: "
            "unexpected stage number occured");
    }
}

pair<BytesShared, size_t> IntermediateNodePaymentTransaction::serializeToBytes()
{
    throw Exception("Not implemented");
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::initOperation()
{
#define COORDINATOR_UUID mMessage->senderUUID()

    info() << "Init. intermediate payment operation. "
              "Initilizer node - (" << COORDINATOR_UUID << ")";

    // Note:
    // Order is necessary:
    // coordinator request must be processed in first order.
    if (! mTrustLines->isNeighbor(mMessage->senderUUID()))
        return processReservationAssumingCoordinatorIsRemoteNode();

    return processReservationAssumingCoordiinatorIsNeighbour();

#undef COORDINATOR_UUID
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::processReservationAssumingCoordiinatorIsNeighbour()
{
    const auto kCoordinator = mMessage->senderUUID();


    info() << "Processing reservation request assuming coordinator is neighbor.";

    if (! mTrustLines->isNeighbor(kCoordinator)) {
        error() << "No trustline to the contractor is present.";
        error() << "This may signal about process/data modification during execution, "
                   "or protocol error of the communicator node.";

        // Request must be ignored
        return exit();
    }


    // note: don't delete this (copy of shared pointer is required)
    const auto incomingAmount = availableIncomingAmount(kCoordinator);
    TrustLineAmount reservationAmount =
        min(mMessage->amount(), *incomingAmount);

    if (0 == reservationAmount) {
        info() << "Max amount against neighbour is " << reservationAmount;
        info() << "No reservation is posible. Stopping.";

        return rejectRequest();
    }

    if (! reserveIncomingAmount(kCoordinator, reservationAmount)) {
        error() << "Can't reserve incoming amount " << reservationAmount;
        error() << "Transaction may not be proceed. Stopping.";

        return rejectRequest();
    }

    info() << "Successfully reserved " << reservationAmount;
    info() << "Waiting for prolongation message or commit command.";
    return acceptRequest();
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::processReservationAssumingCoordinatorIsRemoteNode()
{
    const auto coordinatorUUID = mMessage->senderUUID();
    const auto nextNodeUUID = mMessage->nextNodeInThePathUUID();


    info() << "Init. intermediate payment operation. ";
    info() << "Initialised by the coordinator (" << coordinatorUUID << ")";


    try {
        auto tl = mTrustLines->trustLineReadOnly(nextNodeUUID);
        TrustLineAmount reservationAmount =
                min(mMessage->amount(), *tl->availableAmount());

        if (0 == reservationAmount) {
            info() << "Max amount against neighbour is " << reservationAmount;
            info() << "No reservation is posible. Stopping";

            // TODO: Send cancel message through previous node
            return make_shared<TransactionResult>(
                TransactionState::exit());
        }

        mTrustLines->reserveAmount(
            mMessage->nextNodeInThePathUUID(), UUID(), reservationAmount);

        info() << "Successfully reserved amount on current node side: " << reservationAmount;
        info() << "Waiting for request approving from neighbor";

        sendMessage(
            make_shared<ReserveBalanceRequestMessage>(
                nodeUUID(),
                UUID(),
                reservationAmount,
                nodeUUID()), // TODO: last node UUID may be omitted
            nextNodeUUID);

        // TODO another codes
        return make_shared<TransactionResult>(
            TransactionState::exit());

    } catch (NotFoundError) {
        // TODO: log full transaction info
        error() << "Attempt to reserve amount against non related node occurred";

        // TODO: Send cancel message through previous node
        return make_shared<TransactionResult>(
            TransactionState::exit());

    } catch (exception &e) {
        // TODO: log full transaction info

        error() << "Unexpected error occurred on amount reservation: "
                << e.what()
                << "No amount reservation was done. Operation can't be proceed";

        // TODO: Send cancel message
        return make_shared<TransactionResult>(
            TransactionState::exit());
    }
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::rejectRequest()
{
    sendMessage(
        make_shared<IntermediateNodeReservationResponse>(
            nodeUUID(),
            UUID(),
            IntermediateNodeReservationResponse::Rejected),
        mMessage->senderUUID());

    return exit();
}

TransactionResult::SharedConst IntermediateNodePaymentTransaction::acceptRequest()
{
    sendMessage(
        make_shared<IntermediateNodeReservationResponse>(
            nodeUUID(),
            UUID(),
            IntermediateNodeReservationResponse::Accepted),
        mMessage->senderUUID());

    // TODO: Wait for prolongation message
    return exit();
}


void IntermediateNodePaymentTransaction::deserializeFromBytes(
    BytesShared buffer)
{
    throw Exception("Not implemented");
}

const string IntermediateNodePaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[IntermediateNodePaymentTA: " << UUID().stringUUID() << "] ";

    return s.str();
}

