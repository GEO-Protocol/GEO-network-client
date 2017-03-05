#include "CoordinatorPaymentTransaction.h"


CoordinatorPaymentTransaction::PathStats::PathStats(
    Path::ConstShared path) :

    mPath(path),
    mIntermediateNodesStates(
        path->length()-2, // intermediate nodes count
        ReservationRequestDoesntSent),
    mMaxPathFlow(0)
{}

void CoordinatorPaymentTransaction::PathStats::setNodeState(
    const uint8_t positionInPath,
    const CoordinatorPaymentTransaction::PathStats::NodeState state)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(positionInPath > 0);
    assert(positionInPath <= mIntermediateNodesStates.size());
#endif

    // TODO: add check of state degradation

    // Note: only intermediate nodes are present in mNodesStates,
    // so the -1 is required.
    mIntermediateNodesStates[positionInPath-1] = state;
}

const Path::ConstShared CoordinatorPaymentTransaction::PathStats::path() const
{
    return mPath;
}

/**
 * @returns node to which amount reservation request wasn't set yet,
 * and it's position on the path, relative to the source node on the path.
 * The node is represented by the UUID.
 *
 * @throws NotFoundError - in case if all nodes of this path are already processed.
 */
const pair<NodeUUID, uint8_t> CoordinatorPaymentTransaction::PathStats::nextIntermediateNodeAndPos() const
{
    for (uint8_t i=0; i<mIntermediateNodesStates.size(); ++i) {
        if (mIntermediateNodesStates[i] == PathStats::ReservationRequestDoesntSent) {
            return make_pair(mPath->nodes[i+1], i+1);
        }
    }

    throw NotFoundError(
        "CoordinatorPaymentTransaction::PathStats::nextNodeRequestMustBeSent: "
        "no unprocessed nodes are left");
}

const bool CoordinatorPaymentTransaction::PathStats::reservationRequestSentToAllNodes() const
{
    return mIntermediateNodesStates.at(
        mIntermediateNodesStates.size()-1) != ReservationRequestDoesntSent;
}

/**
 * @returns true if current path sent amount reservation request and
 * is now waiting for the response to it.
 */
const bool CoordinatorPaymentTransaction::PathStats::isWaitingForReservationResponse() const
{
    for (const auto& it: mIntermediateNodesStates) {
        if (it == PathStats::ReservationRequestSent)
            return true;
    }

    return false;
}

const bool CoordinatorPaymentTransaction::PathStats::isReadyToSendNextReservationRequest() const
{
    return !isWaitingForReservationResponse() &&
           !isLastIntermediateNodeProcessed();
}

const bool CoordinatorPaymentTransaction::PathStats::isLastIntermediateNodeProcessed() const
{
    return
        mIntermediateNodesStates[mIntermediateNodesStates.size()-1] !=
            PathStats::ReservationRequestDoesntSent;
}


CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    const NodeUUID &currentNodeUUID,
    CreditUsageCommand::Shared command,
    TrustLinesManager *trustLines,
    Logger *log) :

    BasePaymentTransaction(
        BaseTransaction::CoordinatorPaymentTransaction,
        currentNodeUUID,
        trustLines,
        log),
    mCommand(command),
    mReservationsStage(0),
    mAlreadyReservedAmountOnAllPaths(0),
    mCurrentAmountReservingPathIdentifierIndex(0)
{}

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log):

    BasePaymentTransaction(
        BaseTransaction::CoordinatorPaymentTransaction,
        buffer,
        trustLines,
        log)
{}


TransactionResult::SharedConst CoordinatorPaymentTransaction::run()
{
    switch (mStep) {
        case 1:
            return initTransaction();

        case 2:
            return processReceiverResponse();

        case 3:
            return tryReserveAmounts();

        default:
            throw ValueError(
                "CoordinatorPaymentTransaction::run(): "
                    "invalid transaction step.");
    }

    // TODO: remove this
    return make_shared<TransactionResult>(
        TransactionState::exit());
}


TransactionResult::SharedConst CoordinatorPaymentTransaction::initTransaction()
{
    info() << "Init operation to node (" << mCommand->contractorUUID() << ")";
    info() << "Command UUID: " << mCommand->UUID();
    info() << "Operation amount: " << mCommand->amount();

    if (mCommand->contractorUUID() == nodeUUID()) {
        info() << "Attempt to initialise operation against itself prevented. Canceled.";
        return resultProtocolError();
    }

    // TODO: optimisation
    // Check if total outgoing possibilities of this node
    // are not smaller, than total operation amount.
    // In case if so - there is no reason to begin the operation:
    // current node would not be able to pay such an amount.


    // TODO: Read paths from paths manager.
    // TODO: Ensure paths shuffling
    // TODO: optimisation: if no paths are avaialbe - no operation can be proceed.

    NodeUUID sender = nodeUUID();
    NodeUUID b("13e5cf8c-5834-4e52-b65b-f9281dd1ff01");
    NodeUUID c("13e5cf8c-5834-4e52-b65b-f9281dd1ff02");
    NodeUUID receiver("13e5cf8c-5834-4e52-b65b-f9281dd1ff03");

    auto p1 = make_shared<const Path>(
        Path(sender, receiver, {b, c}));
//    auto p2 = make_shared<const Path>(
//        Path(sender, receiver, {c}));

    addPathForFurtherProcessing(p1);
//    addPathForFurtherProcessing(p2);


    info() << "Collected paths: ";
    for (const auto &identifierAndStats : mPathsStats) {
        info() << "{" << identifierAndStats.second->path()->toString() << "}; ";
    }


    // If there is no one path to the receiver -
    // transaction can't proceed.
    //
    // TODO: load more paths from paths manager.
    if (mPathsStats.empty()) {
        return resultNoPathsError();
    }

    sendMessage(
        make_shared<ReceiverInitPaymentMessage>(
            nodeUUID(),
            UUID(),
            mCommand->amount()),
        mCommand->contractorUUID());

    increaseStepsCounter();
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypes(
            {Message::Payments_ReceiverApprove},
            kMaxMessageTransferLagMSec));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processReceiverResponse()
{
    if (mContext.empty()) {
        info() << "No receiver response was received. Canceled.";

        // TODO: (hsc) retry if no response (but no more than 5 times)
        // TODO: (hsc) in case of retry, deadline interval of previous stage must be divided by 5.
        return resultNoResponseError();
    }

    if (mContext.size() > 1) {
        info() << "Protocol error: more than one message received. Canceled.";
        return resultProtocolError();
    }

    if (mContext.at(0)->typeID() != Message::Payments_ReceiverApprove) {
        info() << "Protocol error: unexpected message type received. Cancel.";
        return resultProtocolError();
    }


    info() << "Receiver response received. Amounts reservation stage started";

    // Removing of already processed message.
    mContext.erase(mContext.cbegin());

    increaseStepsCounter();
    return make_shared<const TransactionResult>(
        TransactionState::flushAndContinue());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::tryReserveAmounts()
{
    switch (mReservationsStage) {
    case 0: {
            initAmountsReservationOnNextPath();
            mReservationsStage += 1;

            // Note:
            // next section must be executed immediately.
            // (no "break" is needed).
        }

    case 1: {
            auto path = currentAmountReservationPathStats();
            if (path->isReadyToSendNextReservationRequest()) {
                return tryReserveNextNodeAmount(path);

            } else if (path->isWaitingForReservationResponse())
                return processRemoteNodeResponse();

            else {
                throw RuntimeError(
                    "Unexpected behaviour occured");
            }
        }

    default:
        throw ValueError(
            "CoordinatorPaymentTransaction::tryBlockAmounts: "
            "unexpected reservations stage occured.");
    }

    return make_shared<const TransactionResult>(
        TransactionState::flushAndContinue());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->resultOK());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultNoPathsError()
{
    return transactionResultFromCommand(
        mCommand->resultNoPaths());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->resultProtocolError());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultNoResponseError()
{
    return transactionResultFromCommand(
        mCommand->resultNoResponse());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultInsufficientFundsError()
{
    return transactionResultFromCommand(
        mCommand->resultInsufficientFundsError());
}

void CoordinatorPaymentTransaction::addPathForFurtherProcessing(
    Path::ConstShared path)
{
    // Preventing paths duplication
    for (const auto &identifierAndStats : mPathsStats) {
        if (identifierAndStats.second->path() == path)
            throw ConflictError("CoordinatorPaymentTransaction::addPathForFurtherProcessing: "
                "duplicated path occured in the transaction.");
    }

    for (;;) {
        // Cylce is needed to prevent possible hashes collison.
        PathUUID identifier = boost::uuids::random_generator()();
        if (mPathsStats.count(identifier) == 0){
            mPathsStats[identifier] = make_unique<PathStats>(path);
            return;
        }
    }
}

void CoordinatorPaymentTransaction::initAmountsReservationOnNextPath()
{
    if (mPathsStats.size() == 0)
        throw NotFoundError(
            "CoordinatorPaymentTransaction::tryBlockAmounts: "
            "no paths are available");

    mCurrentAmountReservingPathIdentifier = mPathsStats.begin()->first;
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::tryReserveNextNodeAmount(
    PathStats* pathStats)
{
    if (mAlreadyReservedAmountOnAllPaths == mCommand->amount()){
        // Internal process error.
        // No next path must be selected.
        // Transaction execution must be cancelled.

        throw RuntimeError(
            "CoordinatorPaymentTransaction::tryReserveNextNodeAmount: "
            "no additional reservation is required");
    }


    /*
     * Nodes scheme:
     *  C - current node;
     *  R - remote node;
     *  S - next node in path after remote one;
     */

    const auto C_UUID = nodeUUID();

    const auto R_UUIDAndPos = pathStats->nextIntermediateNodeAndPos();
    const auto R_UUID = R_UUIDAndPos.first;
    const auto R_PathPosition = R_UUIDAndPos.second;

    const auto S_PathPosition = R_PathPosition + 1;
    const auto S_UUID = pathStats->path()->nodes[S_PathPosition];


    // If next node is first in path -
    // coordinator must reserve amount on it's side first.
    if (R_PathPosition == 1) {

        if (! mTrustLines->isNeighbor(R_UUID)){
            error() << "Invalid path occured. "
                       "Node (" << R_UUID << ") is not listed in first level contractors list.";
            error() << "This may signal about protocol/data manipulations.";

            return tryProcessNextPath();
        }

        info() << "Processing first node in path: (" << R_UUID << ").";

        const auto reservationAmount = availableAmount(R_UUID);
        if (*reservationAmount == 0) {
            // Coordinator has no ability to pay this node.
            // This path can'e be used any more.

            info() << "No amount is available against (" << R_UUID << ")."
                      "Switching to another path.";

            return tryProcessNextPath();
        }

        info() << "Amount reserved locally: " << *reservationAmount;
        return sendReservationRequest(
            pathStats,
            R_UUID,
            R_PathPosition,
            S_UUID,
            *reservationAmount);
    }


    // Next node is not first in path
    auto reservationAmount =
        mCommand->amount() - mAlreadyReservedAmountOnAllPaths;

    if (reservationAmount == 0){
        // Internal process error.
        // No next path must be selected.
        // Transaction execution must be cancelled.

        throw RuntimeError(
            "CoordinatorPaymentTransaction::tryReserveNextNodeAmount: "
            "Reservation amount == 0, but it must be greater than 0. "
            "This may signal about protocol/data modifications during execution.");
    }

    if (reservationAmount > mCommand->amount()) {
        // Internal process error.
        // No next path must be selected.
        // Transaction execution must be cancelled.

        throw RuntimeError(
            "CoordinatorPaymentTransaction::tryReserveNextNodeAmount: "
            "Data overflow prevented. "
            "Already reserved amount is greater than transaction amount. "
            "This may signal about protocol/data modifications during execution.");
    }

    return sendReservationRequest(
        pathStats,
        R_UUID,
        R_PathPosition,
        S_UUID,
        reservationAmount);
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::sendReservationRequest(
    PathStats *pathStats,
    const NodeUUID &remoteNode,
    const byte remoteNodePosition,
    const NodeUUID &nextNodeAfterRemote,
    const TrustLineAmount& amount)
{

#define CURRENT_NODE_UUID nodeUUID()
#define TRANSACTION_UUID UUID()

    sendMessage(
        make_shared<ReserveBalanceRequestMessage>(
            CURRENT_NODE_UUID,
            TRANSACTION_UUID,
            amount,
            nextNodeAfterRemote),
        remoteNode);

    pathStats->setNodeState(
        remoteNodePosition,
        PathStats::ReservationRequestSent);

    info() << "Amount reservation request sent to the node: " << remoteNode.stringUUID();
    info() << "Requested reservation amount is: " << amount;
    info() << "Next path node is : " << nextNodeAfterRemote.stringUUID();

    // Response from te remote node will go throught other nodes in the path.
    // So them would be able to shortage it's reservations (if needed).
    // Total wait timeout must take note of this.
    const auto kTimeout = kMaxMessageTransferLagMSec * remoteNodePosition;

    info() << "The node is " << int(remoteNodePosition) << " in path. "
           << "Timeout for response from it: " << kTimeout << " msecs";

    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypes(
            {Message::Payments_ReserveBalanceResponse},
            kTimeout));
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processRemoteNodeResponse()
{
    if (mContext.empty()) {
        // Remote node doesn't sent a response.
        // Whole path now must be dropped.
        mPathsStats.erase(mPathsStats.cbegin());
        if (mPathsStats.empty()) {

            info() << "No remote node response was received"
                      " and no more paths are available. Stopping";
            return resultInsufficientFundsError();

        } else {

            info() << "No remote node response was received. "
                      "Switching to another path";

            return tryProcessNextPath();
        }
    }

    return resultOK();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::tryProcessNextPath()
{
    try {
        switchToNextPath();
        return tryReserveAmounts();

    } catch (Exception &e) {
        info() << "No another paths are available. Canceling.";
        return resultInsufficientFundsError();
    }
}

CoordinatorPaymentTransaction::PathStats* CoordinatorPaymentTransaction::currentAmountReservationPathStats()
{
    return mPathsStats[mCurrentAmountReservingPathIdentifier].get();
}

void CoordinatorPaymentTransaction::switchToNextPath()
{
    if (mPathsStats.size() > 0)
        mPathsStats.erase(mPathsStats.cbegin());

    if (mPathsStats.size() == 0)
        throw NotFoundError(
            "CoordinatorPaymentTransaction::switchToNextPath: "
            "no paths are available");

    mCurrentAmountReservingPathIdentifier = mPathsStats.begin()->first;
}

///**
// * @brief CoordinatorPaymentTransaction::beginAmountReservation
// *
// * Sends first amount reservation requests to all nodes,
// * that are in first position of a corresponding paths
// * (omitting sender).
// */
//void CoordinatorPaymentTransaction::beginAmountReservation()
//{
//    for (const auto &identifierAndStats : mPathsStats) {
//        auto path = identifierAndStats.second->path();
//        if (path.intermediateNodesCount == 0) {
//            // TODO: Process edge case
//            // Direct payment from sender to receiver.

//            throw Exception(
//                "CoordinatorPaymentTransaction::beginAmountReservation: "
//                "edge case: direct payment!");
//        }

//        auto firtsNodeIdentifier = path.intermediateNodes[0];
//        auto message = make_sahred<ReserveBalanceRequestMessage>(
//            nodeUUID(),
//            UUID(),
//            mcomm)

//        addMessage();
//    }
//}

//TransactionResult::SharedConst CoordinatorPaymentTransaction::processMiddlewareNodesResponse() {

//    if (mContext == nullptr) {
//        // No response was received.
//        // TODO: (hsc) retry if no response (but no more than 5 times)
//        // TODO: (hsc) in case of retry, deadline interval of previous stage must be divided by 5.

//        return transactionResultFromCommand(
//            mCommand->resultRemoteNodeIsInaccessible());
//    }

//    if (mContext->typeID() != Message::Payments_OperationStateMessageType) {
//        // Invalid (unexpected) response was received.
//        throw ValueError(
//            "CoordinatorPaymentTransaction::processReceiverResponse: "
//                "invalid message type received. "
//                "'Payments_OperationStateMessageType' is expected.");
//    }

//    increaseStepsCounter();
//    return make_shared<const TransactionResult>(
//        TransactionState::flushAndContinue());

//}

pair<BytesShared, size_t> CoordinatorPaymentTransaction::serializeToBytes()
{
    throw ValueError("Not implemented");
}

void CoordinatorPaymentTransaction::deserializeFromBytes(BytesShared buffer)
{
    throw ValueError("Not implemented");
}

const string CoordinatorPaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[CoordinatorPaymentTA: " << UUID().stringUUID() << "] ";

    return s.str();
}
