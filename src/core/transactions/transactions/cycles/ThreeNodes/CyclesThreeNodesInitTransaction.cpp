#include "CyclesThreeNodesInitTransaction.h"

CyclesThreeNodesInitTransaction::CyclesThreeNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    RoutingTableManager *routingTable,
    CyclesManager *cyclesManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_ThreeNodesInitTransaction,
        nodeUUID,
        equivalent,
        logger),
    mTrustLinesManager(manager),
    mCyclesManager(cyclesManager),
    mContractorUUID(contractorUUID),
    mRougingTable(routingTable)
{}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::run()
{
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

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::runCollectDataAndSendMessageStage()
{
    debug() << "runCollectDataAndSendMessageStage to " << mContractorUUID;
    if (!mTrustLinesManager->trustLineIsActive(mContractorUUID)) {
        warning() << "TL with contractor is not active";
        return resultDone();
    }
    set<NodeUUID> commonNeighbors = getNeighborsWithContractor();
    if(commonNeighbors.empty()){
        info() << "No common neighbors with: " << mContractorUUID;
        return resultDone();
    }
    sendMessage<CyclesThreeNodesBalancesRequestMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        commonNeighbors);
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultWaitForMessageTypes(
        {Message::Cycles_ThreeNodesBalancesResponse},
        mkStandardConnectionTimeout);
}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::runParseMessageAndCreateCyclesStage()
{
    debug() << "runParseMessageAndCreateCyclesStage";
    if (mContext.empty()){
        info() << "No responses messages are present. Can't create cycles paths;";
        return resultDone();
    }
#ifdef DDEBUG_LOG_CYCLES_BUILDING_POCESSING
    vector<vector<NodeUUID>> ResultCycles;
#endif
    auto message = popNextMessage<CyclesThreeNodesBalancesResponseMessage>();
    for(const auto &nodeUUIDAndBalance : message->commonNodes() ){
        vector<NodeUUID> cycle = {
            nodeUUIDAndBalance,
            mContractorUUID};
        if (mTrustLinesManager->balance(mContractorUUID) > TrustLine::kZeroBalance()) {
            reverse(
                cycle.begin(),
                cycle.end());
        }
        // Path object is common object. For cycle - destination and source node is the same
        const auto cyclePath = make_shared<Path>(
            mNodeUUID,
            mNodeUUID,
            cycle);
#ifdef DDEBUG_LOG_CYCLES_BUILDING_POCESSING
        debug() << "build cycle: " << mNodeUUID << " -> " << mContractorUUID
                << " -> " << nodeUUIDAndBalance << " -> " << mNodeUUID;
#endif
        mCyclesManager->addCycle(
            cyclePath);
#ifdef DDEBUG_LOG_CYCLES_BUILDING_POCESSING
            ResultCycles.push_back(cycle);
#endif
    }

#ifdef DDEBUG_LOG_CYCLES_BUILDING_POCESSING
    debug() << "ResultCyclesCount " << ResultCycles.size();
    for (vector<NodeUUID> KCyclePath: ResultCycles){
        stringstream ss;
        copy(KCyclePath.begin(), KCyclePath.end(), ostream_iterator<NodeUUID>(ss, ","));
        debug() << "CyclePath " << ss.str();
    }
    debug() << "End" << endl;
#endif
    mCyclesManager->closeOneCycle();
    return resultDone();
}

set<NodeUUID> CyclesThreeNodesInitTransaction::getNeighborsWithContractor()
{
    set<NodeUUID> ownNeighbors, commonNeighbors;
    const auto kBalanceToContractor = mTrustLinesManager->balance(mContractorUUID);
    if (kBalanceToContractor == TrustLine::kZeroBalance()) {
        return commonNeighbors;
    }

    for (const auto &kNodeUUIDAndTrustLine: mTrustLinesManager->trustLines()){
        const auto kTL = kNodeUUIDAndTrustLine.second;
        if (kTL->state() != TrustLine::Active) {
            continue;
        }
        if (kBalanceToContractor < TrustLine::kZeroBalance()) {
            if (kTL->balance() > TrustLine::kZeroBalance())
                ownNeighbors.insert(kNodeUUIDAndTrustLine.first);
        }
        else if (kTL->balance() < TrustLine::kZeroBalance())
            ownNeighbors.insert(kNodeUUIDAndTrustLine.first);
    }
    const auto contractorNeighbors = mRougingTable->secondLevelContractorsForNode(
        mContractorUUID);
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

const string CyclesThreeNodesInitTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesThreeNodesInitTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
