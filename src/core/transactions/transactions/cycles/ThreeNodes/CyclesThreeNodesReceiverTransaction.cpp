#include "CyclesThreeNodesReceiverTransaction.h"

CyclesThreeNodesReceiverTransaction::CyclesThreeNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesThreeNodesBalancesRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_ThreeNodesReceiverTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mTrustLinesManager(manager),
    mRequestMessage(message)
{}

TransactionResult::SharedConst CyclesThreeNodesReceiverTransaction::run()
{
    if (!mTrustLinesManager->trustLineIsActive(mRequestMessage->senderUUID)) {
        warning() << "TL with requested node " << mRequestMessage->senderUUID << " is not active";
        return resultDone();
    }
    const auto kNeighbors = mRequestMessage->neighbors();
    const auto kContractorBalance = mTrustLinesManager->balance(mRequestMessage->senderUUID);

    if (kContractorBalance == TrustLine::kZeroBalance()) {
        return resultDone();
    }

    vector<NodeUUID> commonNeighbors;

    bool searchDebtors = true;
    if (kContractorBalance > TrustLine::kZeroBalance())
        searchDebtors = false;

    TrustLineBalance stepNodeBalance;
    for (const auto &kNodeUUID: kNeighbors) {
        if (!mTrustLinesManager->trustLineIsActive(kNodeUUID)) {
            continue;
        }
        stepNodeBalance = mTrustLinesManager->balance(kNodeUUID);
        if ((searchDebtors and stepNodeBalance > TrustLine::kZeroBalance())
            or (not searchDebtors and (stepNodeBalance < TrustLine::kZeroBalance())))
            commonNeighbors.push_back(
                kNodeUUID);
    }
    if (!commonNeighbors.empty())
        sendMessage<CyclesThreeNodesBalancesResponseMessage>(
            mRequestMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            currentTransactionUUID(),
            commonNeighbors);
    return resultDone();
}

const string CyclesThreeNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesThreeNodesReceiverTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}

