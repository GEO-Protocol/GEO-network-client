#include "CyclesFourNodesInitTransaction.h"

CyclesFourNodesInitTransaction::CyclesFourNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &debtorContractorUUID,
    const NodeUUID &creditorContractorUUID,
    TrustLinesManager *manager,
    RoutingTablesHandler *routingTablesHandler,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesInitTransaction,
        nodeUUID),
    mTrustLinesManager(manager),
    mLogger(logger),
    mRoutingTablesHandler(routingTablesHandler),
    mDebtorContractorUUID(debtorContractorUUID),
    mCreditorContractorUUID(creditorContractorUUID)
{}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::run() {
    switch (mStep) {
        case Stages::CollectDataAndSendMessage:
            return runCollectDataAndSendMessageStage();

        case Stages::ParseMessageAndCreateCycles:
            return runParseMessageAndCreateCyclesStage();

        default:
            throw RuntimeError(
                "CycleThreeNodesInitTransaction::run(): "
                "Invalid transaction step.");
    }
}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::runCollectDataAndSendMessageStage() {

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
        mkStandardConnectionTimeout);
}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage() {
    if (mContext.size() != 2){
        mLogger->error("CyclesFourNodesInitTransaction:"
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
        mLogger->info("CyclesFourNodesInitTransaction:"
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

set<NodeUUID> CyclesFourNodesInitTransaction::getCommonNeighborsForDebtorAndCreditorNodes() {
    const auto creditorsNeighbors = mRoutingTablesHandler->routingTable2Level()->allDestinationsForSource(
        mCreditorContractorUUID);
    const auto debtorsNeighbors = mRoutingTablesHandler->routingTable2Level()->allDestinationsForSource(
        mCreditorContractorUUID);

    set<NodeUUID> commonNeighbors;
    set_intersection(
        creditorsNeighbors.begin(),
        creditorsNeighbors.end(),
        debtorsNeighbors.begin(),
        debtorsNeighbors.end(),
        std::inserter(
            commonNeighbors,
            commonNeighbors.begin()));

    return commonNeighbors;
}
