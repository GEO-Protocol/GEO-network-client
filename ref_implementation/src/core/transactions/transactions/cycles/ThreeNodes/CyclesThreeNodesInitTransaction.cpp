/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "CyclesThreeNodesInitTransaction.h"

CyclesThreeNodesInitTransaction::CyclesThreeNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    TrustLinesManager *manager,
    RoutingTableManager *roughtingTable,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_ThreeNodesInitTransaction,
        nodeUUID,
        logger),
    mTrustLinesManager(manager),
    mCyclesManager(cyclesManager),
    mContractorUUID(contractorUUID),
    mStorageHandler(storageHandler),
    mRoughtingTable(roughtingTable)
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

// todo : need used topology from MaxFlowCalculationTrustLineManager
set<NodeUUID> CyclesThreeNodesInitTransaction::getNeighborsWithContractor()
{
    const auto kBalanceToContractor = mTrustLinesManager->balance(mContractorUUID);
    const TrustLineBalance kZeroBalance = 0;
    auto ioTransactions = mStorageHandler->beginTransaction();
    const auto contractorNeighbors = mRoughtingTable->secondLevelContractorsForNode(mContractorUUID);

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
    if(neighbors.size() == 0){
        info() << "No common neighbors with: " << mContractorUUID;
        return resultDone();
    }
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
        info() << "No responses messages are present. Can't create cycles paths;";
        return resultDone();
    }
#ifdef DDEBUG_LOG_CYCLES_BUILDING_POCESSING
    vector<vector<NodeUUID>> ResultCycles;
#endif
    auto message = static_pointer_cast<CyclesThreeNodesBalancesResponseMessage>(*mContext.begin());
    const auto neighborsAndBalances = message->NeighborsAndBalances();
    for(const auto &nodeUUIDAndBalance : neighborsAndBalances ){
        vector<NodeUUID> cycle = {
            mContractorUUID,
            nodeUUIDAndBalance};
        reverse(cycle.begin(), cycle.end());
        // Path object is common object. For cycle - destination and sourse node is the same
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

const string CyclesThreeNodesInitTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesThreeNodesInitTransactionTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
