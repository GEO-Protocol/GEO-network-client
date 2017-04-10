#include "CyclesThreeNodesReceiverTransaction.h"

CyclesThreeNodesReceiverTransaction::CyclesThreeNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesThreeNodesBalancesRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_ThreeNodesReceiverTransaction,
        message->transactionUUID(),
        nodeUUID,
        logger),
    mTrustLinesManager(manager),
    mLogger(logger),
    mRequestMessage(message)
{}

TransactionResult::SharedConst CyclesThreeNodesReceiverTransaction::run() {
    const auto kNeighbors = mRequestMessage->Neighbors();
    stringstream ss;
    // Create message and reserve memory for neighbors
    cout << "_______________________________" << endl;
    cout << "CyclesThreeNodesReceiverTransaction::run(): Transaction UUID " << UUID() << endl;
    const auto kMessage = make_shared<CyclesThreeNodesBalancesResponseMessage>(
        mNodeUUID,
        UUID(),
        kNeighbors.size());
    cout <<  "CyclesThreeNodesReceiverTransaction::run(): Sender UUID \n" << mRequestMessage->senderUUID().stringUUID() << endl;
    cout << "CyclesThreeNodesReceiverTransaction::run(): Neighbors from message: \n" << endl;
    copy(kNeighbors.begin(), kNeighbors.end(), ostream_iterator<NodeUUID>(ss, "\n"));
    cout << ss.str() << endl;
    const auto kContractorBalance = mTrustLinesManager->balance(mRequestMessage->senderUUID());
    const TrustLineBalance kZeroBalance = 0;

    bool searchDebtors = true;
    if (kContractorBalance > kZeroBalance)
        searchDebtors = false;

    //
    TrustLineBalance stepNodeBalance;
    for (const auto &kNodeUUID: kNeighbors) {
        stepNodeBalance = mTrustLinesManager->balance(kNodeUUID);
        cout << "StepNode UUID " << (stepNodeBalance < kZeroBalance) <<  (not searchDebtors) << endl;
        if ((searchDebtors and stepNodeBalance > kZeroBalance)
            or (not searchDebtors and (stepNodeBalance < kZeroBalance)))
            kMessage->addNeighborUUIDAndBalance(
                    kNodeUUID);
    }
    if (kMessage->NeighborsAndBalances().size() > 0)
        sendMessage(mRequestMessage->senderUUID(), kMessage);
    return finishTransaction();
}



