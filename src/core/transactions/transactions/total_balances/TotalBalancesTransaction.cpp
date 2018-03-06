#include "TotalBalancesTransaction.h"

TotalBalancesTransaction::TotalBalancesTransaction(
    NodeUUID &nodeUUID,
    TotalBalancesCommand::Shared command,
    TrustLinesManager *manager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::TotalBalancesTransactionType,
        nodeUUID,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

TotalBalancesCommand::Shared TotalBalancesTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst TotalBalancesTransaction::run()
{
    TrustLineAmount totalIncomingTrust = 0;
    TrustLineAmount totalTrustUsedByContractor = 0;
    TrustLineAmount totalOutgoingTrust = 0;
    TrustLineAmount totalTrustUsedBySelf = 0;

    // if contractor is gateway, than outgoing trust amount is equal balance on this TL
    for (auto &nodeUUIDAndTrustLine : mTrustLinesManager->trustLines()) {
        if (find(mCommand->gateways().begin(),
                 mCommand->gateways().end(),
                 nodeUUIDAndTrustLine.first) == mCommand->gateways().end()) {
            totalOutgoingTrust += nodeUUIDAndTrustLine.second->outgoingTrustAmount();
        } else {
            auto totalOutgoingTrustUsedShared = nodeUUIDAndTrustLine.second->usedAmountByContractor();
            totalOutgoingTrust += *totalOutgoingTrustUsedShared.get();
        }
        totalIncomingTrust += nodeUUIDAndTrustLine.second->incomingTrustAmount();
        auto totalOutgoingTrustUsedShared = nodeUUIDAndTrustLine.second->usedAmountByContractor();
        totalTrustUsedByContractor += *totalOutgoingTrustUsedShared.get();
        auto totalIncomingTrustUsedShared = nodeUUIDAndTrustLine.second->usedAmountBySelf();
        totalTrustUsedBySelf += *totalIncomingTrustUsedShared.get();
    }

    return resultOk(
        totalIncomingTrust,
        totalTrustUsedByContractor,
        totalOutgoingTrust,
        totalTrustUsedBySelf);
}

TransactionResult::SharedConst TotalBalancesTransaction::resultOk(
    const TrustLineAmount &totalIncomingTrust,
    const TrustLineAmount &totalTrustUsedByContractor,
    const TrustLineAmount &totalOutgoingTrust,
    const TrustLineAmount &totalTrustUsedBySelf)
{
    stringstream s;
    s << totalIncomingTrust << kTokensSeparator << totalTrustUsedByContractor
      << kTokensSeparator << totalOutgoingTrust << kTokensSeparator << totalTrustUsedBySelf;
    string totalBalancesStrResult = s.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            totalBalancesStrResult));
}

const string TotalBalancesTransaction::logHeader() const
{
    stringstream s;
    s << "[TotalBalancesTA: " << currentNodeUUID() << " " << mEquivalent << "]";
    return s.str();
}
