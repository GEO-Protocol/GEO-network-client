#include "CyclesThreeNodesInitTransaction.h"

CyclesThreeNodesInitTransaction::CyclesThreeNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    TrustLinesManager *manager,
    RoutingTablesHandler *routingTablesHandler,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_ThreeNodesInitTransaction,
        nodeUUID),
    mRoutingTablesHandler(routingTablesHandler),
    mTrustLinesManager(manager),
    mLogger(logger),
    mContractorUUID(contractorUUID)
{}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::run() {
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

set<NodeUUID> CyclesThreeNodesInitTransaction::getNeighborsWithContractor() {
    const auto kBalanceToContractor = mTrustLinesManager->balance(mContractorUUID);
    const TrustLineBalance kZeroBalance = 0;

    auto contractorNeighbors =
        mRoutingTablesHandler->routingTable2Level()->allDestinationsForSource(
        mContractorUUID);

    set<NodeUUID> ownNeighbors, commonNeighbors;
    for (const auto &kNodeUUIDAndTrustLine: mTrustLinesManager->trustLines()){

        const auto kTL = kNodeUUIDAndTrustLine.second;
        if (kBalanceToContractor < kZeroBalance) {
            if (kTL->balance() > kZeroBalance)
                ownNeighbors.insert(kNodeUUIDAndTrustLine.first);
        }
        else
            if (kTL->balance() < kZeroBalance)
                ownNeighbors.insert(kNodeUUIDAndTrustLine.first);

        set_intersection(
            ownNeighbors.begin(),
            ownNeighbors.end(),
            contractorNeighbors.begin(),
            contractorNeighbors.end(),
            std::inserter(
                commonNeighbors,
                commonNeighbors.begin()));
    }

    return commonNeighbors;
}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::runCollectDataAndSendMessageStage() {

    set<NodeUUID> neighbors = getNeighborsWithContractor();
    sendMessage<ThreeNodesBalancesRequestMessage>(
        mContractorUUID,
        mNodeUUID,
        mTransactionUUID,
        neighbors);

    mStep = Stages::ParseMessageAndCreateCycles;
    return resultWaitForMessageTypes(
        {Message::Cycles_ThreeNodesBalancesResponseMessage},
        kStandardConnectionTimeout);
}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::runParseMessageAndCreateCyclesStage() {

    if (mContext.size() != 1){
        return finishTransaction();
    }
    auto message = static_pointer_cast<ThreeNodesBalancesResponseMessage>(*mContext.begin());
    const auto neighborsAndBalances = message->NeighborsAndBalances();
    for(const auto &nodeUUIDAndBalance : neighborsAndBalances ){
        vector<NodeUUID> cycle = {
            mNodeUUID,
            mContractorUUID,
            nodeUUIDAndBalance.first};
    }
    //            todo run transaction to close cycle
    return finishTransaction();
}
