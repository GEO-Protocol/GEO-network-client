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

    const auto firstMessage = static_pointer_cast<CyclesFourNodesBalancesResponseMessage>(*mContext.begin());
    const auto secondMessage = static_pointer_cast<CyclesFourNodesBalancesResponseMessage>(*mContext.end());
    const auto firstContractorUUID = firstMessage->senderUUID();
    const auto secondContractorUUID = secondMessage->senderUUID();
    const TrustLineBalance zeroBalance = 0;

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

    TrustLineBalance stepMaxFlow;

    for (auto &kNodeUUIDSecondMessage: firstMessage->NeighborsUUID()){
        for (auto &nodeAndBalanceSecond: secondMessage->NeighborsUUID()){
            if (nodeAndBalanceSecond == kNodeUUIDSecondMessage){
                vector<NodeUUID> stepPath = {
                    mNodeUUID,
                    mDebtorContractorUUID,
                    nodeAndBalanceSecond,
                    mCreditorContractorUUID};
                // Run transaction to close cycle
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
