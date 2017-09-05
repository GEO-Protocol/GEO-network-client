#include "TotalBalancesFromRemoutNodeTransaction.h"

TotalBalancesFromRemoutNodeTransaction::TotalBalancesFromRemoutNodeTransaction(
    NodeUUID &nodeUUID,
    TotalBalancesRemouteNodeCommand::Shared command,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::InitiateTotalBalancesFromRemoutNodeTransactionType,
        nodeUUID,
        logger),
    mCommand(command)
{}

TotalBalancesRemouteNodeCommand::Shared TotalBalancesFromRemoutNodeTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::run()
{
    switch (mStep) {
        case Stages::SendRequestForRemouteNode:
            if (mCommand->contractorUUID() == currentNodeUUID()) {
                warning() << "Attempt to initialise operation against itself was prevented. Canceled.";
                return resultProtocolError();
            }
            sendMessageToRemoteNode();
            mStep = Stages::GetResponseFromRemouteNode;
            return waitingForResponseState();
        case Stages::GetResponseFromRemouteNode:
            if (mContext.empty()) {
                return resultRemoteNodeIsInaccessible();
            }
            return getRemouteNodeTotalBalances();
        default:
            throw ValueError("FindPathTransaction::run: "
                                 "wrong value of mStep");
    }
}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::getRemouteNodeTotalBalances()
{
    if (mContext.size() == 1) {
        auto responseMessage = *mContext.begin();
        if (responseMessage->typeID() == Message::MessageType::TotalBalance_Response) {
            TotalBalancesResultMessage::Shared response = static_pointer_cast<TotalBalancesResultMessage>(
                responseMessage);
            return resultOk(
                response->totalIncomingTrust(),
                response->totalTrustUsedByContractor(),
                response->totalOutgoingTrust(),
                response->totalTrustUsedBySelf());
        }
        warning() << "getRemouteNodeTotalBalances: unexpected message type";
        return resultProtocolError();
    } else {
        throw ConflictError("TotalBalancesFromRemoutNodeTransaction::checkTransactionContext: "
                                "Unexpected context size.");
    }
}

void TotalBalancesFromRemoutNodeTransaction::sendMessageToRemoteNode()
{
    sendMessage<InitiateTotalBalancesMessage>(
        mCommand->contractorUUID(),
        mNodeUUID,
        currentTransactionUUID());
}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::waitingForResponseState()
{
    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageType::TotalBalance_Response},
            kConnectionTimeout));
}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::resultOk(
    const TrustLineAmount &totalIncomingTrust,
    const TrustLineAmount &totalTrustUsedByContractor,
    const TrustLineAmount &totalOutgoingTrust,
    const TrustLineAmount &totalTrustUsedBySelf)
{
    stringstream s;
    s << totalIncomingTrust << "\t" << totalTrustUsedByContractor << "\t" << totalOutgoingTrust << "\t" << totalTrustUsedBySelf;
    string totalBalancesStrResult = s.str();
    return transactionResultFromCommand(mCommand->responseOk(totalBalancesStrResult));
}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::resultRemoteNodeIsInaccessible()
{
    return transactionResultFromCommand(
        mCommand->responseRemoteNodeIsInaccessible());
}

TransactionResult::SharedConst TotalBalancesFromRemoutNodeTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

const string TotalBalancesFromRemoutNodeTransaction::logHeader() const
{
    stringstream s;
    s << "[TotalBalancesFromRemoutNodeTA: " << currentTransactionUUID() << "]";
    return s.str();
}