#include "TotalBalancesFromRemoutNodeTransaction.h"

TotalBalancesFromRemoutNodeTransaction::TotalBalancesFromRemoutNodeTransaction(
    NodeUUID &nodeUUID,
    TotalBalancesRemouteNodeCommand::Shared command,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::InitiateTotalBalancesFromRemoutNodeTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mRequestCounter(0) {}

TotalBalancesRemouteNodeCommand::Shared TotalBalancesFromRemoutNodeTransaction::command() const {

    return mCommand;
}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::run() {

    info() << "run\t" << UUID();

    if (!mContext.empty()) {
        return checkTransactionContext();

    } else {
        if (mRequestCounter < kMaxRequestsCount) {
            sendMessageToRemoteNode();
            increaseRequestsCounter();

        } else {
            return resultRemoteNodeIsInaccessible();
        }

    }
    return waitingForResponseState();

}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::checkTransactionContext() {

    info() << "context size\t" << mContext.size();
    if (mExpectationResponsesCount == mContext.size()) {
        auto responseMessage = *mContext.begin();

        if (responseMessage->typeID() == Message::MessageTypeID::TotalBalancesResultMessageType) {
            TotalBalancesResultMessage::Shared response = static_pointer_cast<TotalBalancesResultMessage>(
                    responseMessage);

            return resultOk(
                response->totalIncomingTrust(),
                response->totalTrustUsedByContractor(),
                response->totalOutgoingTrust(),
                response->totalTrustUsedBySelf());
        }

        return resultRemoteNodeIsInaccessible();

    } else {
        throw ConflictError("TotalBalancesFromRemoutNodeTransaction::checkTransactionContext: "
                                    "Unexpected context size.");
    }
}

void TotalBalancesFromRemoutNodeTransaction::sendMessageToRemoteNode() {

    sendMessage<InitiateTotalBalancesMessage>(
        mCommand->contractorUUID(),
        mNodeUUID,
        UUID());
}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::waitingForResponseState() {

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

void TotalBalancesFromRemoutNodeTransaction::increaseRequestsCounter() {

    mRequestCounter += 1;
    info() << "increaseRequestsCounter\t" << mRequestCounter;
}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::resultOk(
        const TrustLineAmount &totalIncomingTrust,
        const TrustLineAmount &totalTrustUsedByContractor,
        const TrustLineAmount &totalOutgoingTrust,
        const TrustLineAmount &totalTrustUsedBySelf) {

    stringstream s;
    s << totalIncomingTrust << "\t" << totalTrustUsedByContractor << "\t" << totalOutgoingTrust << "\t" << totalTrustUsedBySelf;
    string totalBalancesStrResult = s.str();
    info() << "resultOk\t" << totalBalancesStrResult;
    return transactionResultFromCommand(mCommand->responseOk(totalBalancesStrResult));
}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::resultRemoteNodeIsInaccessible() {

    info() << "resultRemoteNodeIsInaccessible";
    return transactionResultFromCommand(
            mCommand->responseRemoteNodeIsInaccessible());
}

const string TotalBalancesFromRemoutNodeTransaction::logHeader() const
{
    stringstream s;
    s << "[TotalBalancesFromRemoutNodeTA]";

    return s.str();
}