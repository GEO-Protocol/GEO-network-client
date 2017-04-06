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

    set<NodeUUID> neighbors = commonNeighborsForDebtorAndCreditorNodes();
    sendMessage<CyclesFourNodesBalancesRequestMessage>(
        mDebtorContractorUUID,
        mNodeUUID,
        mTransactionUUID,
        neighbors);

    sendMessage<CyclesFourNodesBalancesRequestMessage>(
        mCreditorContractorUUID,
        mNodeUUID,
        mTransactionUUID,
        neighbors);

    mStep = Stages::ParseMessageAndCreateCycles;
    return resultWaitForMessageTypes(
        {Message::Cycles_FourNodesBalancesResponseMessage},
        mkStandardConnectionTimeout);
}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage() {
    if (mContext.size() != 2) {
        error() << "CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage: "
                   "Responses messages count not equals to 2; "
                   "Can't create cycles;";

        return finishTransaction();
    }

    auto firstMessage = static_pointer_cast<CyclesFourNodesBalancesResponseMessage>(*mContext.begin());
    auto secondMessage = static_pointer_cast<CyclesFourNodesBalancesResponseMessage>(*mContext.end());
    auto firstContractorUUID = firstMessage->senderUUID();
    auto secondContractorUUID = secondMessage->senderUUID();
    TrustLineBalance zeroBalance = 0;

    TrustLineBalance firstContractorBalance = mTrustLinesManager->balance(firstContractorUUID);
    TrustLineBalance secondContractorBalance = mTrustLinesManager->balance(secondContractorUUID);

    // In case if some payment operation was done and balances on the nodes was changed -
    // this check prevents redundant cycles closing operations.
    if ((firstContractorBalance > zeroBalance and secondContractorBalance > zeroBalance) or
        (firstContractorBalance < zeroBalance and secondContractorBalance < zeroBalance) or
        (firstContractorBalance == zeroBalance and secondContractorBalance == zeroBalance)) {

        info() << "CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage: "
                  "Balances between initiator node and (" << firstContractorUUID <<  "), or "
                  "between initiator node and (" << secondContractorUUID << ") was changed. "
                  "Cannot create cycles.";

        return finishTransaction();
    }

    const bool kFirstContractorIsCreditor = firstContractorBalance < zeroBalance;
    if (kFirstContractorIsCreditor)
        firstContractorBalance = firstContractorBalance * (-1);
    else
        secondContractorBalance = secondContractorBalance * (-1);

    TrustLineBalance maxFlow = min(firstContractorBalance, secondContractorBalance);
    TrustLineBalance stepMaxFlow;
    vector<pair<vector<NodeUUID>, TrustLineBalance>> ResultCycles;

    for (auto &nodeAndBalanceFirst: firstMessage->NeighborsBalances()){
        if (kFirstContractorIsCreditor)
            nodeAndBalanceFirst.second = nodeAndBalanceFirst.second * (-1);
        for (auto &nodeAndBalanceSecond: secondMessage->NeighborsBalances()){
            if (nodeAndBalanceSecond.first == nodeAndBalanceFirst.first){
                if (not kFirstContractorIsCreditor)
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

set<NodeUUID> CyclesFourNodesInitTransaction::commonNeighborsForDebtorAndCreditorNodes() {
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
