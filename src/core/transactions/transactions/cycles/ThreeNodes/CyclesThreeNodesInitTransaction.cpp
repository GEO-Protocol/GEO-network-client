#include "CyclesThreeNodesInitTransaction.h"

CyclesThreeNodesInitTransaction::CyclesThreeNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    TrustLinesManager *manager,
    RoutingTablesHandler *routingTablesHandler,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_ThreeNodesInitTransaction,
        nodeUUID,
        logger),
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
    const auto contractorNeighbors =
        mRoutingTablesHandler->routingTable2Level()->allDestinationsForSource(
        mContractorUUID);
    cout << "CyclesThreeNodesInitTransaction::mContractorUUID " << mContractorUUID << endl;
    stringstream ss;
    copy(contractorNeighbors.begin(), contractorNeighbors.end(), ostream_iterator<NodeUUID>(ss, " "));
    cout << ss.str() << endl;
    cout << "CyclesThreeNodesInitTransaction::End" << endl;
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
    }
    set_intersection(
            ownNeighbors.begin(),
            ownNeighbors.end(),
            contractorNeighbors.begin(),
            contractorNeighbors.end(),
            std::inserter(
                    commonNeighbors,
                    commonNeighbors.begin()));

    return commonNeighbors;
}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::runCollectDataAndSendMessageStage() {

    set<NodeUUID> neighbors = getNeighborsWithContractor();
    sendMessage<CyclesThreeNodesBalancesRequestMessage>(
        mContractorUUID,
        mNodeUUID,
        UUID(),
        neighbors);
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultWaitForMessageTypes(
        {Message::Cycles_ThreeNodesBalancesReceiverMessage},
        mkStandardConnectionTimeout);
}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::runParseMessageAndCreateCyclesStage() {
    if (mContext.size() != 1){
        return finishTransaction();
    }
    #ifdef TESTS
    vector<vector<NodeUUID>> ResultCycles;
    #endif
    auto message = static_pointer_cast<CyclesThreeNodesBalancesResponseMessage>(*mContext.begin());
    const auto neighborsAndBalances = message->NeighborsAndBalances();
    for(const auto &nodeUUIDAndBalance : neighborsAndBalances ){
        vector<NodeUUID> cycle = {
            mNodeUUID,
            mContractorUUID,
            nodeUUIDAndBalance};
        auto sCycle = make_shared<vector<NodeUUID>>(cycle);
        closeCycleSignal(sCycle);
        #ifdef TESTS
            ResultCycles.push_back(cycle);
        #endif
    }

    #ifdef TESTS
    cout << "CyclesThreeNodesInitTransaction::ResultCyclesCount " << to_string(ResultCycles.size()) << endl;
    for (vector<NodeUUID> KCyclePath: ResultCycles){
        stringstream ss;
        copy(KCyclePath.begin(), KCyclePath.end(), ostream_iterator<NodeUUID>(ss, ","));
        cout << "CyclesThreeNodesInitTransaction::CyclePath " << ss.str() << endl;
    }
    cout << "CyclesThreeNodesInitTransaction::End" << endl;
    #endif

    return finishTransaction();
}
