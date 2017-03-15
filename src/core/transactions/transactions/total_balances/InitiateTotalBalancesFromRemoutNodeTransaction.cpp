#include "InitiateTotalBalancesFromRemoutNodeTransaction.h"

InitiateTotalBalancesFromRemoutNodeTransaction::InitiateTotalBalancesFromRemoutNodeTransaction(
    NodeUUID &nodeUUID,
    TotalBalancesRemouteNodeCommand::Shared command,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::InitiateTotalBalancesFromRemoutNodeTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mRequestCounter(0) {}

TotalBalancesRemouteNodeCommand::Shared InitiateTotalBalancesFromRemoutNodeTransaction::command() const {

    return mCommand;
}

TransactionResult::SharedConst InitiateTotalBalancesFromRemoutNodeTransaction::run() {

    info() << "run\t";

    if (!mContext.empty()) {
        return checkTransactionContext();

    } else {
        if (mRequestCounter < kMaxRequestsCount) {
            sendMessageToRemoteNode();
            increaseRequestsCounter();

        } else {
            return noResponseResult();
        }

    }
    return waitingForResponseState();

}

TransactionResult::SharedConst InitiateTotalBalancesFromRemoutNodeTransaction::checkTransactionContext() {

    if (mExpectationResponsesCount == mContext.size()) {
        auto responseMessage = *mContext.begin();

        if (responseMessage->typeID() == Message::MessageTypeID::TotalBalancesResultMessageType) {
            TotalBalancesResultMessage::Shared response = static_pointer_cast<TotalBalancesResultMessage>(
                    responseMessage);

            return resultOk(
                response->totalIncomingTrust(),
                response->totalIncomingTrustUsed(),
                response->totalOutgoingTrust(),
                response->totalOutgoingTrustUsed());
        }

        return unexpectedErrorResult();

    } else {
        throw ConflictError("InitiateTotalBalancesFromRemoutNodeTransaction::checkTransactionContext: "
                                    "Unexpected context size.");
    }
}

void InitiateTotalBalancesFromRemoutNodeTransaction::sendMessageToRemoteNode() {

    sendMessage<InitiateTotalBalancesMessage>(
        mCommand->contractorUUID(),
        mNodeUUID);
}

TransactionResult::SharedConst InitiateTotalBalancesFromRemoutNodeTransaction::waitingForResponseState() {

    info() << "waitingForResponseState";
    TransactionState *transactionState = new TransactionState(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(kConnectionTimeout * 1000)),
        Message::MessageTypeID::TotalBalancesResultMessageType,
        false);

    return transactionResultFromState(
        TransactionState::SharedConst(
            transactionState));
}

void InitiateTotalBalancesFromRemoutNodeTransaction::increaseRequestsCounter() {

    mRequestCounter += 1;
    info() << "increaseRequestsCounter\t" << mRequestCounter;
}

TransactionResult::SharedConst InitiateTotalBalancesFromRemoutNodeTransaction::resultOk(
        const TrustLineAmount &totalIncomingTrust,
        const TrustLineAmount &totalIncomingTrustUsed,
        const TrustLineAmount &totalOutgoingTrust,
        const TrustLineAmount &totalOutgoingTrustUsed) {

    stringstream s;
    s << totalIncomingTrust << "\t" << totalIncomingTrustUsed << "\t" << totalOutgoingTrust << "\t" << totalOutgoingTrustUsed;
    string totalBalancesStrResult = s.str();
    info() << "resultOk\t" << totalBalancesStrResult;
    return transactionResultFromCommand(mCommand->resultOk(totalBalancesStrResult));
}

TransactionResult::SharedConst InitiateTotalBalancesFromRemoutNodeTransaction::noResponseResult() {

    info() << "noResponseResult";
    return transactionResultFromCommand(
            mCommand->resultNoResponse());
}

TransactionResult::SharedConst InitiateTotalBalancesFromRemoutNodeTransaction::unexpectedErrorResult() {

    return transactionResultFromCommand(
            mCommand->unexpectedErrorResult());
}

const string InitiateTotalBalancesFromRemoutNodeTransaction::logHeader() const
{
    stringstream s;
    s << "[InitiateTotalBalancesFromRemoutNodeTA]";

    return s.str();
}