#include "CyclesThreeNodesInitTransaction.h"

CyclesThreeNodesInitTransaction::CyclesThreeNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    TrustLinesManager *manager,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_ThreeNodesInitTransaction,
        nodeUUID,
        logger),
    mTrustLinesManager(manager),
    mCyclesManager(cyclesManager),
    mContractorUUID(contractorUUID),
    mStorageHandler(storageHandler)
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

set<NodeUUID> CyclesThreeNodesInitTransaction::getNeighborsWithContractor()
{
    const auto kBalanceToContractor = mTrustLinesManager->balance(mContractorUUID);
    const TrustLineBalance kZeroBalance = 0;
    const auto contractorNeighbors =
        mStorageHandler->routingTablesHandler()->neighborsOfOnRT2(
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

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::runCollectDataAndSendMessageStage()
{
    debug() << "runCollectDataAndSendMessageStage to " << mContractorUUID;
    set<NodeUUID> neighbors = getNeighborsWithContractor();
    sendMessage<CyclesThreeNodesBalancesRequestMessage>(
        mContractorUUID,
        mNodeUUID,
        currentTransactionUUID(),
        neighbors);
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultWaitForMessageTypes(
        {Message::Cycles_ThreeNodesBalancesResponse},
        mkStandardConnectionTimeout);
}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::runParseMessageAndCreateCyclesStage()
{
    debug() << "runParseMessageAndCreateCyclesStage";
    if (mContext.size() != 1){
        return resultDone();
    }
    #ifdef TESTS
    vector<vector<NodeUUID>> ResultCycles;
    #endif
    auto message = static_pointer_cast<CyclesThreeNodesBalancesResponseMessage>(*mContext.begin());
    const auto neighborsAndBalances = message->NeighborsAndBalances();
    for(const auto &nodeUUIDAndBalance : neighborsAndBalances ){
        vector<NodeUUID> cycle = {
            mContractorUUID,
            nodeUUIDAndBalance};
        // Path object is common object. For cycle - destination and sourse node is the same
        const auto cyclePath = make_shared<Path>(
            mNodeUUID,
            mNodeUUID,
            cycle);
        debug() << "build cycle: " << mNodeUUID << " -> " << mContractorUUID
                << " -> " << nodeUUIDAndBalance << " -> " << mNodeUUID;
        mCyclesManager->addCycle(
            cyclePath);
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
    mCyclesManager->closeOneCycle();
    return resultDone();
}

const string CyclesThreeNodesInitTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesThreeNodesInitTransactionTA: " << currentTransactionUUID() << "] ";

    return s.str();
}
