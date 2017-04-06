#include "CyclesSixNodesInitTransaction.h"

const BaseTransaction::TransactionType CyclesSixNodesInitTransaction::transactionType() const{
    return BaseTransaction::TransactionType::Cycles_SixNodesInitTransaction;
}

TransactionResult::SharedConst CyclesSixNodesInitTransaction::runCollectDataAndSendMessagesStage() {
    const auto firstLevelNodes = mTrustLinesManager->firstLevelNeighborsWithNoneZeroBalance();
    vector<NodeUUID> path;
    path.push_back(mNodeUUID);
    for(const auto &value: firstLevelNodes){
        sendMessage<CyclesSixNodesInBetweenMessage>(
            value,
            path
        );
    }
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultAwaikAfterMilliseconds(mkWaitingForResponseTime);
}

CyclesSixNodesInitTransaction::CyclesSixNodesInitTransaction(
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    Logger *logger) :
    CyclesBaseFiveSixNodesInitTransaction(
        BaseTransaction::TransactionType::Cycles_SixNodesInitTransaction,
        nodeUUID,
        manager,
        logger)
{}


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CyclesSixNodesInitTransaction::runParseMessageAndCreateCyclesStage() {

    TrustLineBalance zeroBalance = 0;
    CycleMap mCreditors;
    vector <NodeUUID> stepPath;
    TrustLineBalance creditorsStepFlow;
    for(const auto &mess: mContext){
        auto message = static_pointer_cast<CyclesSixNodesBoundaryMessage>(mess);
        stepPath = message->Path();
//  It has to be exactly nodes count in path
        if (stepPath.size() != 3)
            continue;
        creditorsStepFlow = mTrustLinesManager->balance(stepPath[1]);
//  If it is Debtor branch - skip it
        if (creditorsStepFlow > zeroBalance)
            continue;
//  Check all Boundary Nodes and add it to map if all checks path
        for (auto &nodeUUIDAndBalance: message->BoundaryNodes()){
//  Prevent loop on cycles path
            if (nodeUUIDAndBalance.first == stepPath.front())
                continue;
            mCreditors.insert(make_pair(
                nodeUUIDAndBalance.first,
                make_pair(stepPath, (-1) * max(creditorsStepFlow, nodeUUIDAndBalance.second))));

        }
    }

//    Create Cycles comparing BoundaryMessages data with debtors map
    ResultVector mCycles;
//     stepCyclePath;
    TrustLineBalance debtorsStepFlow;
    TrustLineBalance commonStepMaxFlow;
    for(const auto &mess: mContext) {
        auto message = static_pointer_cast<CyclesSixNodesBoundaryMessage>(mess);
        debtorsStepFlow = mTrustLinesManager->balance(message->Path()[1]);
//  If it is Creditors branch - skip it
        if (debtorsStepFlow < zeroBalance)
            continue;
        stepPath = message->Path();
//  It has to be exactly nodes count in path
        if (stepPath.size() != 3)
            continue;
        for (auto &NodeUUIDAndBalance: message->BoundaryNodes()) {
//  Prevent loop on cycles path
            if (NodeUUIDAndBalance.first == stepPath.front())
                continue;
            mapIter m_it, s_it;
            pair <mapIter, mapIter> keyRange = mCreditors.equal_range(NodeUUIDAndBalance.first);
            for (s_it = keyRange.first; s_it != keyRange.second; ++s_it) {
                if ((s_it->second->first[2] == stepPath[1]) or (s_it->second.first[1] == stepPath[2]))
                    continue;
//  Find minMax flow between 3 value. 1 in map. 1 in boundaryNodes. 1 we get from creditor first node in path
                commonStepMaxFlow = min(min(s_it->second.second, debtorsStepFlow), NodeUUIDAndBalance.second);
                vector <NodeUUID> stepCyclePath = {stepPath[0],
                                                   stepPath[1],
                                                   stepPath[2],
                                                   NodeUUIDAndBalance.first,
                                                   s_it->second.first[2],
                                                   s_it->second.first[1]};
                //    Todo run cycles
//                mCycles.push_back(make_pair(stepCyclePath, commonStepMaxFlow));
                stepCyclePath.clear();
            }
        }
    }
    mContext.clear();

    return finishTransaction();
}
