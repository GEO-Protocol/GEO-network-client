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

TransactionResult::SharedConst TotalBalancesTransaction::run()
{
    TrustLineAmount totalIncomingTrust = 0;
    TrustLineAmount totalTrustUsedByContractor = 0;
    TrustLineAmount totalOutgoingTrust = 0;
    TrustLineAmount totalTrustUsedBySelf = 0;

    // if contractor is gateway, than outgoing trust amount is equal balance on this TL
    for (auto &nodeIDAndTrustLine : mTrustLinesManager->trustLines()) {
        if (!nodeIDAndTrustLine.second->isContractorGateway()) {
            totalOutgoingTrust += nodeIDAndTrustLine.second->outgoingTrustAmount();
        } else {
            auto totalOutgoingTrustUsedShared = nodeIDAndTrustLine.second->usedAmountByContractor();
            totalOutgoingTrust += *totalOutgoingTrustUsedShared.get();
        }
        totalIncomingTrust += nodeIDAndTrustLine.second->incomingTrustAmount();
        auto totalOutgoingTrustUsedShared = nodeIDAndTrustLine.second->usedAmountByContractor();
        totalTrustUsedByContractor += *totalOutgoingTrustUsedShared.get();
        auto totalIncomingTrustUsedShared = nodeIDAndTrustLine.second->usedAmountBySelf();
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
