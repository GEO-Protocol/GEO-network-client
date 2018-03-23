#include "CyclesSixNodesInitTransaction.h"

CyclesSixNodesInitTransaction::CyclesSixNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    CyclesManager *cyclesManager,
    Logger &logger) :
    CyclesBaseFiveSixNodesInitTransaction(
        BaseTransaction::TransactionType::Cycles_SixNodesInitTransaction,
        nodeUUID,
        equivalent,
        manager,
        cyclesManager,
        logger)
{}

const BaseTransaction::TransactionType CyclesSixNodesInitTransaction::transactionType() const
{
    return BaseTransaction::TransactionType::Cycles_SixNodesInitTransaction;
}

TransactionResult::SharedConst CyclesSixNodesInitTransaction::runCollectDataAndSendMessagesStage()
{
    debug() << "runCollectDataAndSendMessagesStage";
    const auto firstLevelNodes = mTrustLinesManager->firstLevelNeighborsWithNoneZeroBalance();
    vector<NodeUUID> path;
    path.push_back(mNodeUUID);
    for(const auto &kNodeUUID: firstLevelNodes){
        sendMessage<CyclesSixNodesInBetweenMessage>(
            kNodeUUID,
            mEquivalent,
            path);
    }
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultAwakeAfterMilliseconds(mkWaitingForResponseTime);
}

TransactionResult::SharedConst CyclesSixNodesInitTransaction::runParseMessageAndCreateCyclesStage()
{
    debug() << "runParseMessageAndCreateCyclesStage";
    if (mContext.empty()) {
        info() << "No responses messages are present. Can't create cycles paths;";
        return resultDone();
    }
    CycleMap mCreditors;
    TrustLineBalance creditorsStepFlow;
    for(const auto &mess: mContext){
        auto message = static_pointer_cast<CyclesSixNodesBoundaryMessage>(mess);
        const auto stepPath = make_shared<vector<NodeUUID>>(message->Path());
        //  It has to be exactly nodes count in path
        if (stepPath->size() != 3)
            continue;
        creditorsStepFlow = mTrustLinesManager->balance((*stepPath)[1]);
        //  If it is Debtor branch - skip it
        if (creditorsStepFlow > TrustLine::kZeroBalance())
            continue;
        //  Check all Boundary Nodes and add it to map if all checks path
        for (auto &nodeUUID: message->BoundaryNodes()){
            //  Prevent loop on cycles path
            if (nodeUUID == stepPath->front())
                continue;
            mCreditors.insert(make_pair(
                nodeUUID,
                stepPath));
        }
    }

    //  Create Cycles comparing BoundaryMessages data with debtors map
    TrustLineBalance debtorsStepFlow;
    TrustLineBalance commonStepMaxFlow;
    vector<NodeUUID> stepPath;
#ifdef DDEBUG_LOG_CYCLES_BUILDING_POCESSING
    vector<vector<NodeUUID>> ResultCycles;
#endif
    for(const auto &mess: mContext) {
        auto message = static_pointer_cast<CyclesSixNodesBoundaryMessage>(mess);
        debtorsStepFlow = mTrustLinesManager->balance(message->Path()[1]);
        //  If it is Creditors branch - skip it
        if (debtorsStepFlow < TrustLine::kZeroBalance())
            continue;
        stepPath = message->Path();
        //  It has to be exactly nodes count in path
        if (stepPath.size() != 3)
            continue;

        for (const auto &kNodeUUID: message->BoundaryNodes()) {
            //  Prevent loop on cycles path
            if (kNodeUUID == stepPath.front())
                continue;
            auto NodeUIIDAndPathRange = mCreditors.equal_range(kNodeUUID);
            for (auto NodeUIDandPairOfPathandBalace = NodeUIIDAndPathRange.first;
                 NodeUIDandPairOfPathandBalace != NodeUIIDAndPathRange.second; ++NodeUIDandPairOfPathandBalace) {
                if (((*NodeUIDandPairOfPathandBalace->second)[2] == stepPath[1])
                    or ((*NodeUIDandPairOfPathandBalace->second)[1] == stepPath[2]))
                    continue;
                //  Find minMax flow between 3 value. 1 in map. 1 in boundaryNodes. 1 we get from creditor first node in path
                vector<NodeUUID> stepCyclePath = {
                                                  stepPath[1],
                                                  stepPath[2],
                                                  kNodeUUID,
                                                  (*(NodeUIDandPairOfPathandBalace->second))[2],
                                                  (*(NodeUIDandPairOfPathandBalace->second))[1]};
                stringstream ss;
                copy(stepCyclePath.begin(), stepCyclePath.end(), ostream_iterator<NodeUUID>(ss, ","));
                debug() << "runParseMessageAndCreateCyclesStage::ResultPath " << ss.str();
                const auto cyclePath = make_shared<Path>(
                    mNodeUUID,
                    mNodeUUID,
                    stepCyclePath);
                mCyclesManager->addCycle(
                    cyclePath);
#ifdef DDEBUG_LOG_CYCLES_BUILDING_POCESSING
                ResultCycles.push_back(stepCyclePath);
#endif
            }
        }
    }
#ifdef DDEBUG_LOG_CYCLES_BUILDING_POCESSING
    debug() << "CyclesFiveNodesInitTransaction::ResultCyclesCount " << to_string(ResultCycles.size());
    for (vector<NodeUUID> KCyclePath: ResultCycles){
        stringstream ss;
        copy(KCyclePath.begin(), KCyclePath.end(), ostream_iterator<NodeUUID>(ss, ","));
        debug() << "CyclesFiveNodesInitTransaction::CyclePath " << ss.str();
    }
    debug() << "CyclesFiveNodesInitTransaction::End";
#endif
    mContext.clear();
    mCyclesManager->closeOneCycle();
    return resultDone();
}

const string CyclesSixNodesInitTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesSixNodesInitTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}