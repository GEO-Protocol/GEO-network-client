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
        mTrustLinesManager(manager) {}

TotalBalancesCommand::Shared TotalBalancesTransaction::command() const {

    return mCommand;
}

TransactionResult::SharedConst TotalBalancesTransaction::run() {

    info() << "run\t";

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

    return resultOk(
            totalIncomingTrust,
            totalIncomingTrustUsed,
            totalOutgoingTrust,
            totalOutgoingTrustUsed);
    //return make_shared<TransactionResult>(TransactionState::exit());

}

TransactionResult::SharedConst TotalBalancesTransaction::resultOk(
        TrustLineAmount &totalIncomingTrust,
        TrustLineAmount &totalIncomingTrustUsed,
        TrustLineAmount &totalOutgoingTrust,
        TrustLineAmount &totalOutgoingTrustUsed) {

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
