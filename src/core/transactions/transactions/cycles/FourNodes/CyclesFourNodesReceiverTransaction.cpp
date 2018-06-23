#include "CyclesFourNodesReceiverTransaction.h"

CyclesFourNodesReceiverTransaction::CyclesFourNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFourNodesNegativeBalanceRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesReceiverTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mTrustLinesManager(manager),
    mRequestMessage(message),
    mNegativeCycleBalance(true)
{}

CyclesFourNodesReceiverTransaction::CyclesFourNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFourNodesPositiveBalanceRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesReceiverTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mTrustLinesManager(manager),
    mRequestMessage(message),
    mNegativeCycleBalance(false)
{}

TransactionResult::SharedConst CyclesFourNodesReceiverTransaction::run()
{
    if (!mTrustLinesManager->trustLineIsPresent(mRequestMessage->contractor())) {
        warning() << "Contractor node " << mRequestMessage->contractor() << " is not a neighbor";
        return resultDone();
    }

    if (mNegativeCycleBalance) {
        buildSuitableDebtorsForCycleNegativeBalance();
    } else {
        buildSuitableDebtorsForCyclePositiveBalance();
    }

    if (!mSuitableNodes.empty()) {
        sendMessage<CyclesFourNodesBalancesResponseMessage>(
            mRequestMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            currentTransactionUUID(),
            mSuitableNodes);
    }

    return resultDone();
}

void CyclesFourNodesReceiverTransaction::buildSuitableDebtorsForCycleNegativeBalance()
{
    auto creditorBalance = mTrustLinesManager->balance(mRequestMessage->contractor());
    if (creditorBalance <= TrustLine::kZeroBalance()) {
        info() << "Not positive balance with contractor node";
        return;
    }

    for (const auto &debtor : mRequestMessage->checkedNodes()) {
        if (!mTrustLinesManager->trustLineIsPresent(debtor)) {
            warning() << "Checked node " << debtor << " is not a neighbor";
            continue;
        }
        if (!mTrustLinesManager->trustLineIsActive(debtor)) {
            warning() << "TL with checked node " << debtor << " is not active";
            continue;
        }
        if (mTrustLinesManager->balance(debtor) < TrustLine::kZeroBalance()) {
            mSuitableNodes.push_back(debtor);
        }
    }
}

void CyclesFourNodesReceiverTransaction::buildSuitableDebtorsForCyclePositiveBalance()
{
    auto creditorBalance = mTrustLinesManager->balance(mRequestMessage->contractor());
    if (creditorBalance >= TrustLine::kZeroBalance()) {
        info() << "Not negative balance with contractor node";
        return;
    }

    for (const auto &debtor : mRequestMessage->checkedNodes()) {
        if (!mTrustLinesManager->trustLineIsPresent(debtor)) {
            warning() << "Checked node " << debtor << " is not a neighbor";
            continue;
        }
        if (!mTrustLinesManager->trustLineIsActive(debtor)) {
            warning() << "TL with checked node " << debtor << " is not active";
            continue;
        }
        if (mTrustLinesManager->balance(debtor) > TrustLine::kZeroBalance()) {
            mSuitableNodes.push_back(debtor);
        }
    }
}

const string CyclesFourNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFourNodesReceiverTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}