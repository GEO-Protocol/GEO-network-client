#include "CycleFourNodesInitTransaction.h"


CycleFourNodesInitTransaction::CycleFourNodesInitTransaction(TransactionsScheduler *scheduler)
        :UniqueTransaction(BaseTransaction::TransactionType::CycleFourNodesInitTransaction, scheduler) {

}

CycleFourNodesInitTransaction::CycleFourNodesInitTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &debtorContractorUUID,
        const NodeUUID &creditorContractorUUID,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger)
        : UniqueTransaction(BaseTransaction::TransactionType::CycleFourNodesInitTransaction, nodeUUID, scheduler),
          mTrustLinesManager(manager),
          mlogger(logger),
          mStorageHandler(storageHandler),
          mDebtorContractorUUID(debtorContractorUUID),
          mCreditorContractorUUID(creditorContractorUUID)
{

}

TransactionResult::SharedConst CycleFourNodesInitTransaction::run() {
    switch (mStep){
        case Stages::CollectDataAndSendMessage:
            return runCollectDataAndSendMessageStage();
        case Stages::ParseMessageAndCreateCycles:
            return runParseMessageAndCreateCyclesStage();
        default:
            throw RuntimeError(
                "CycleThreeNodesInitTransaction::run(): "
                    "invalid transaction step.");

    }
}

TransactionResult::SharedConst CycleFourNodesInitTransaction::runCollectDataAndSendMessageStage() {

    set<NodeUUID> neighbors = getCommonNeighborsForDebtorAndCreditorNodes();
    sendMessage<FourNodesBalancesRequestMessage>(
        mDebtorContractorUUID,
        mNodeUUID,
        mTransactionUUID,
        neighbors
    );
    sendMessage<FourNodesBalancesRequestMessage>(
        mCreditorContractorUUID,
        mNodeUUID,
        mTransactionUUID,
        neighbors
    );
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultWaitForMessageTypes(
        {Message::FourNodesBalancesResponseMessage},
        kStandardConnectionTimeout);
}

TransactionResult::SharedConst CycleFourNodesInitTransaction::runParseMessageAndCreateCyclesStage() {
    if (mContext.size() != 2){
        mlogger->error("CycleFourNodesInitTransaction:"
                           "Responses Messages count Not equal 2;"
                           "Can not create Cycles;");
        return finishTransaction();
        }
    auto firstMessage = static_pointer_cast<FourNodesBalancesResponseMessage>(*mContext.begin());
    auto secondMessage = static_pointer_cast<FourNodesBalancesResponseMessage>(*mContext.end());
    auto firstContractorUUID = firstMessage->senderUUID();
    auto secondContractorUUID = secondMessage->senderUUID();
    TrustLineBalance zeroBalance = 0;
    TrustLineBalance firstContractorBalance = mTrustLinesManager->balance(firstContractorUUID);
    TrustLineBalance secondContractorBalance = mTrustLinesManager->balance(secondContractorUUID);
    if ((firstContractorBalance < zeroBalance and secondContractorBalance > zeroBalance) or
        (firstContractorBalance > zeroBalance and secondContractorBalance < zeroBalance))
    {
        mlogger->info("CycleFourNodesInitTransaction:"
                          "Balances was changed. Cannot create Cycles");
        return finishTransaction();
    }
    bool isFirstContractorCreditor = false;
    if (firstContractorBalance < zeroBalance) {
        firstContractorBalance = firstContractorBalance * (-1);
        isFirstContractorCreditor = true;
    } else {
        secondContractorBalance = secondContractorBalance * (-1);
    }
    TrustLineBalance maxFlow = min(firstContractorBalance, secondContractorBalance);
    TrustLineBalance stepMaxFlow;
    vector<pair<vector<NodeUUID>, TrustLineBalance>> ResultCycles;

    for (auto &nodeAndBalanceFirst: firstMessage->NeighborsBalances()){
        if (isFirstContractorCreditor)
            nodeAndBalanceFirst.second = nodeAndBalanceFirst.second * (-1);
        for (auto &nodeAndBalanceSecond: secondMessage->NeighborsBalances()){
            if (nodeAndBalanceSecond.first == nodeAndBalanceFirst.first){
                if (not isFirstContractorCreditor)
                    nodeAndBalanceSecond.second = nodeAndBalanceSecond.second * (-1);
                stepMaxFlow = min(maxFlow, min(nodeAndBalanceSecond.second, nodeAndBalanceFirst.second));
                vector<NodeUUID> stepPath = {
                    mNodeUUID,
                    mDebtorContractorUUID,
                    nodeAndBalanceSecond.first,
                    mCreditorContractorUUID};
                ResultCycles.push_back(make_pair(
                   stepPath,
                   stepMaxFlow
                ));
            }
        }
    }
    return TransactionResult::SharedConst();
}

set<NodeUUID> CycleFourNodesInitTransaction::getCommonNeighborsForDebtorAndCreditorNodes() {
    set<NodeUUID> creditorsNeighbors = mStorageHandler->routingTablesHandler()->routingTable2Level()->allDestinationsForSource(
        mCreditorContractorUUID);
    set<NodeUUID> debtorsNeighbors = mStorageHandler->routingTablesHandler()->routingTable2Level()->allDestinationsForSource(
        mCreditorContractorUUID);

    set<NodeUUID> commonNeighbors;
    set_intersection(creditorsNeighbors.begin(), creditorsNeighbors.end(),
                     debtorsNeighbors.begin(), debtorsNeighbors.end(),
                     std::inserter(commonNeighbors, commonNeighbors.begin()));

    return commonNeighbors;
}
