#include "CoordinatorPaymentTransaction.h"

PaymentPath::PaymentPath(
    Path& path,
    Identifier identifier) :

    mPath(path),
    mIdentifier(identifier),
    mIsAmountBlocksSent(false),
    mMaxCommonCapabilities(numeric_limits<TrustLineAmount>::max()) {
}

const bool PaymentPath::wasUsedForAmountReservation() const {
    return mIsAmountBlocksSent;
}

/**
 *
 * @throws bad_alloc
 */
void PaymentPathsHandler::add(
    Path& path) {

    // TODO: generate unique sha256 identifier
    // from path nodes + random salt
    auto identifier = mPaths.size() + 1;

    mPaths.push_back(
        make_unique<PaymentPath>(
                    path, identifier));
}

/**
 * @returns payment path that may be used for amount reservation.
 *
 * @throws NotFoundError in case if no path is awailable.
 */
const PaymentPath& PaymentPathsHandler::nextNotReservedPaymentPath() const
{
    if (mPaths.empty()) {
        throw NotFoundError(
            "PaymentPathsHandler::nextNotBlockedPath: "
            "no paths are awailable.");
    }

    for (auto const& path : mPaths) {
        if (path->wasUsedForAmountReservation())
            continue;

        return *path;
    }

    throw NotFoundError(
        "PaymentPathsHandler::nextNotBlockedPath: "
        "no more paths are awailable.");
}

const bool PaymentPathsHandler::empty() const {
    return mPaths.empty();
}

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    NodeUUID &currentNodeUUID,
    CreditUsageCommand::Shared command,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        BaseTransaction::CoordinatorPaymentTransaction, currentNodeUUID),
    mCommand(command),
    mTrustLines(trustLines),
    mLog(log){
}

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        BaseTransaction::CoordinatorPaymentTransaction),
    mTrustLines(trustLines),
    mLog(log){

    throw ValueError("Not implemented");
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::run() {

    switch (mStep) {
        case 1:
            return initTransaction();

        case 2:
            return processReceiverResponse();

        case 3:
            return tryBlockAmounts();

        default:
            throw ValueError(
                "CoordinatorPaymentTransaction::run(): "
                    "invalid transaction step.");
    }

    // TODO: remove this
    return make_shared<TransactionResult>(
        TransactionState::exit());
}


TransactionResult::SharedConst CoordinatorPaymentTransaction::initTransaction() {

#ifdef TRANSACTIONS_LOG
    {
        auto info = mLog->info(logHeader());
        info << "Init operation to node (" << mCommand->contractorUUID() << ")";
    }
    {
        auto info = mLog->info(logHeader());
        info << "Command UUID: " << mCommand->UUID();
    }
    {
        auto info = mLog->info(logHeader());
        info << "Operation amount: " << mCommand->amount();
    }
#endif

    if (mCommand->contractorUUID() == nodeUUID()) {
#ifdef TRANSACTIONS_LOG
        {
            auto info = mLog->info(logHeader());
            info << "Attempt to initialise operation against itself prevented. Cancel.";
        }
#endif

        return resultProtocolError();
    }

    // TODO: optimisation
    // Check if total outgoing possibilities of this node
    // are not smaller, than total operation amount.
    // In case if so - there is no reason to begin the operation:
    // current node would not be able to pay such an amount.

    // TODO: read paths from paths manager.
    // TODO: optimisation: if no paths are avaialbe - no operation can be proceed.

    NodeUUID sender = nodeUUID();
    NodeUUID receiver("fa9b02d4-cea1-448a-aa62-bfc50f2947a5");
    NodeUUID a("96fadf03-aa7f-4ada-9862-73534460b4c9");
    NodeUUID b("695060e8-9b86-4725-ab93-8d249c50ce08");

    Path p1(sender, receiver, {a});
    Path p2(sender, receiver, {b});

    mPaymentPaths.add(p1);
    mPaymentPaths.add(p2);

#ifdef TRANSACTIONS_LOG
    {
        auto info = mLog->info(logHeader());
        info << "Collected paths: ["
                 << "{" << p1.toString() << "}; "
                 << "{" << p2.toString() << "}; "
             << "]";
    }
#endif

    // If there is no one path to the receiver -
    // transaction can't proceed.
    //
    // TODO: load more paths from paths manager.
    if (mPaymentPaths.empty()) {
        return resultNoPaths();
    }

    // Sending request to the receiver for the approve
    auto message =
        make_shared<ReceiverInitPaymentMessage>(
            nodeUUID(),
            UUID(),
            mCommand->amount());

    addMessage(
        message,
        mCommand->contractorUUID());

    increaseStepsCounter();

    const auto maxReceiverResponseTime = 2000; // msec
    auto state = TransactionState::waitForMessageTypes(
        {Message::Payments_ReceiverApprove}, maxReceiverResponseTime);
    return make_shared<TransactionResult>(state);
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::processReceiverResponse() {

    if (mContext.empty()) {
        // No response was received.
        // TODO: (hsc) retry if no response (but no more than 5 times)
        // TODO: (hsc) in case of retry, deadline interval of previous stage must be divided by 5.

#ifdef TRANSACTIONS_LOG
    {
        auto info = mLog->info(logHeader());
        info << "No receiver response was received. Cancel.";
    }
#endif

        return transactionResultFromCommand(
            mCommand->resultNoResponse());
    }

    if (mContext.size() > 1) {
        // Only one message is expected.
        // In case if more than one message was received -
        // than, it seems that contractor node doesn't follows the protocol.

#ifdef TRANSACTIONS_LOG
    {
        auto info = mLog->info(logHeader());
        info << "Protocol error: more than one message received. Cancel.";
    }
#endif

        return transactionResultFromCommand(
            mCommand->resultProtocolError());
    }

    if (mContext.at(0)->typeID() != Message::Payments_ReceiverApprove) {
        // Invalid (unexpected) response was received.

#ifdef TRANSACTIONS_LOG
    {
        auto info = mLog->info(logHeader());
        info << "Protocol error: unexpected message type received. Cancel.";
    }
#endif

        return transactionResultFromCommand(
            mCommand->resultProtocolError());
    }

    increaseStepsCounter();
    return make_shared<const TransactionResult>(
        TransactionState::flushAndContinue());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::tryBlockAmounts()
{
    // Try lock first available path

//    auto path = mPaymentPaths.

    return make_shared<const TransactionResult>(
        TransactionState::flushAndContinue());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultOK() {
    return transactionResultFromCommand(
        mCommand->resultOK());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultNoPaths() {
    return transactionResultFromCommand(
        mCommand->resultNoPaths());
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultProtocolError() {
    return transactionResultFromCommand(
        mCommand->resultProtocolError());
}

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

pair<BytesShared, size_t> CoordinatorPaymentTransaction::serializeToBytes() {
    throw ValueError("Not implemented");
}

void CoordinatorPaymentTransaction::deserializeFromBytes(BytesShared buffer) {
    throw ValueError("Not implemented");
}

const string CoordinatorPaymentTransaction::logHeader() const
{
    stringstream s;
    s << "[CoordinatorPaymentTA: " << UUID().stringUUID() << "] ";

    return s.str();
}

//void CoordinatorPaymentTransaction::tryBlockAmountsOnIntermediateNodes() {

//    for (auto paymentPath : mPa)
//}


