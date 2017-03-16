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
    TrustLineAmount totalIncomingTrustUsed = 0;
    TrustLineAmount totalOutgoingTrust = 0;
    TrustLineAmount totalOutgoingTrustUsed = 0;

    for (auto &nodeUUIDAndTrustLine : mTrustLinesManager->trustLines()) {
        totalIncomingTrust += nodeUUIDAndTrustLine.second->incomingTrustAmount();
        auto totalIncomingTrustUsedShared = nodeUUIDAndTrustLine.second->usedIncomingAmount();
        totalIncomingTrustUsed += *totalIncomingTrustUsedShared.get();
        totalOutgoingTrust += nodeUUIDAndTrustLine.second->outgoingTrustAmount();
        auto totalOutgoingTrustUsedShared = nodeUUIDAndTrustLine.second->usedOutgoingAmount();
        totalOutgoingTrustUsed += *totalOutgoingTrustUsedShared.get();
    }

    if (mCommand != nullptr) {
        info() << "internal command";
        return resultOk(
            totalIncomingTrust,
            totalIncomingTrustUsed,
            totalOutgoingTrust,
            totalOutgoingTrustUsed);
    }
    if (mMessage != nullptr) {
        info() << "external message\t" << totalIncomingTrust << "\t" << totalIncomingTrustUsed
               << "\t" << totalOutgoingTrust << "\t" << totalOutgoingTrustUsed;
        info() << "transactionUUID\t" << mMessage->transactionUUID();
        sendMessage<TotalBalancesResultMessage>(
            mMessage->senderUUID(),
            mNodeUUID,
            mMessage->transactionUUID(),
            totalIncomingTrust,
            totalIncomingTrustUsed,
            totalOutgoingTrust,
            totalOutgoingTrustUsed);
        return make_shared<TransactionResult>(TransactionState::exit());
    }
    info() << "something wrong: command and message are nulls";
    return make_shared<TransactionResult>(TransactionState::exit());
}

TransactionResult::SharedConst TotalBalancesTransaction::resultOk(
    const TrustLineAmount &totalIncomingTrust,
    const TrustLineAmount &totalIncomingTrustUsed,
    const TrustLineAmount &totalOutgoingTrust,
    const TrustLineAmount &totalOutgoingTrustUsed) {

    stringstream s;
    s << totalIncomingTrust << "\t" << totalIncomingTrustUsed << "\t" << totalOutgoingTrust << "\t" << totalOutgoingTrustUsed;
    string totalBalancesStrResult = s.str();
    return transactionResultFromCommand(mCommand->resultOk(totalBalancesStrResult));
}

const string TotalBalancesTransaction::logHeader() const
{
    stringstream s;
    s << "[TotalBalancesTA]";

    return s.str();
}
