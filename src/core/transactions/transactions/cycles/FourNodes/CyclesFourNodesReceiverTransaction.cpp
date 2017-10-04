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
        logger),
    mTrustLinesManager(manager),
    mRequestMessage(message)
{}

TransactionResult::SharedConst CyclesFourNodesReceiverTransaction::run()
{
    const auto kDebtorNeighbor = mRequestMessage->debtor();
    const auto kCreditorNeighbor = mRequestMessage->creditor();

    auto zeroTrustLine = TrustLineBalance(0);

    if(mTrustLinesManager->balance(kDebtorNeighbor) < zeroTrustLine)
        return resultDone();

    if(mTrustLinesManager->balance(kCreditorNeighbor) > zeroTrustLine)
        return resultDone();

    sendMessage<CyclesFourNodesBalancesResponseMessage>(
        mRequestMessage->senderUUID,
        mNodeUUID,
        currentTransactionUUID()
    );

    return resultDone();
}

const string CyclesFourNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFourNodesReceiverTransactionTA: " << currentTransactionUUID() << "] ";
    return s.str();
}