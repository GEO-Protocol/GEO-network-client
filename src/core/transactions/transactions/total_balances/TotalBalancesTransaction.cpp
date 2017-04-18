#include "TotalBalancesTransaction.h"

TotalBalancesTransaction::TotalBalancesTransaction(
    NodeUUID &nodeUUID,
    TotalBalancesCommand::Shared command,
    TrustLinesManager *manager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::TotalBalancesTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mMessage(nullptr),
    mTrustLinesManager(manager) {}

TotalBalancesTransaction::TotalBalancesTransaction(
    NodeUUID &nodeUUID,
    InitiateTotalBalancesMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger):

    BaseTransaction(
        BaseTransaction::TransactionType::TotalBalancesTransactionType,
        nodeUUID,
        logger),
    mCommand(nullptr),
    mMessage(message),
    mTrustLinesManager(manager) {}

TotalBalancesCommand::Shared TotalBalancesTransaction::command() const {

    return mCommand;
}

InitiateTotalBalancesMessage::Shared TotalBalancesTransaction::message() const {

    return  mMessage;
}

TransactionResult::SharedConst TotalBalancesTransaction::run() {

    info() << "run\t";

    info() << "run\t" << mTrustLinesManager->trustLines().size();
    TrustLineAmount totalIncomingTrust = 0;
    TrustLineAmount totalTrustUsedByContractor = 0;
    TrustLineAmount totalOutgoingTrust = 0;
    TrustLineAmount totalTrustUsedBySelf = 0;

    for (auto &nodeUUIDAndTrustLine : mTrustLinesManager->trustLines()) {
        totalIncomingTrust += nodeUUIDAndTrustLine.second->incomingTrustAmount();
        auto totalIncomingTrustUsedShared = nodeUUIDAndTrustLine.second->usedAmountByContractor();
        totalTrustUsedByContractor += *totalIncomingTrustUsedShared.get();
        totalOutgoingTrust += nodeUUIDAndTrustLine.second->outgoingTrustAmount();
        auto totalOutgoingTrustUsedShared = nodeUUIDAndTrustLine.second->usedAmountBySelf();
        totalTrustUsedBySelf += *totalOutgoingTrustUsedShared.get();
    }

    if (mCommand != nullptr) {
        info() << "internal command";
        return resultOk(
            totalIncomingTrust,
            totalTrustUsedByContractor,
            totalOutgoingTrust,
            totalTrustUsedBySelf);
    }
    if (mMessage != nullptr) {
        info() << "external message\t" << totalIncomingTrust << "\t" << totalTrustUsedByContractor
               << "\t" << totalOutgoingTrust << "\t" << totalTrustUsedBySelf;
        info() << "transactionUUID\t" << mMessage->transactionUUID();
        sendMessage<TotalBalancesResultMessage>(
            mMessage->senderUUID,
            mNodeUUID,
            mMessage->transactionUUID(),
            totalIncomingTrust,
            totalTrustUsedByContractor,
            totalOutgoingTrust,
            totalTrustUsedBySelf);
        return make_shared<TransactionResult>(TransactionState::exit());
    }
    info() << "something wrong: command and message are nulls";
    return make_shared<TransactionResult>(TransactionState::exit());
}

TransactionResult::SharedConst TotalBalancesTransaction::resultOk(
    const TrustLineAmount &totalIncomingTrust,
    const TrustLineAmount &totalTrustUsedByContractor,
    const TrustLineAmount &totalOutgoingTrust,
    const TrustLineAmount &totalTrustUsedBySelf) {

    stringstream s;
    s << totalIncomingTrust << "\t" << totalTrustUsedByContractor << "\t" << totalOutgoingTrust << "\t" << totalTrustUsedBySelf;
    string totalBalancesStrResult = s.str();
    return transactionResultFromCommand(mCommand->resultOk(totalBalancesStrResult));
}

const string TotalBalancesTransaction::logHeader() const
{
    stringstream s;
    s << "[TotalBalancesTA]";

    return s.str();
}
