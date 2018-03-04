#include "CyclesFourNodesReceiverTransaction.h"

CyclesFourNodesReceiverTransaction::CyclesFourNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFourNodesBalancesRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesReceiverTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mTrustLinesManager(manager),
    mRequestMessage(message)
{}

TransactionResult::SharedConst CyclesFourNodesReceiverTransaction::run()
{
    const auto kDebtorNeighbor = mRequestMessage->debtor();
    const auto kCreditorNeighbor = mRequestMessage->creditor();

    if(mTrustLinesManager->balance(kDebtorNeighbor) <= TrustLine::kZeroBalance())
        return resultDone();

    if(mTrustLinesManager->balance(kCreditorNeighbor) >= TrustLine::kZeroBalance())
        return resultDone();

    sendMessage<CyclesFourNodesBalancesResponseMessage>(
        mRequestMessage->senderUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID());

    return resultDone();
}

const string CyclesFourNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFourNodesReceiverTransactionTA: " << currentTransactionUUID() << "] ";
    return s.str();
}