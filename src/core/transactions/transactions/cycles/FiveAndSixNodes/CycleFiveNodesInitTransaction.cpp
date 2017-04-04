#include "CycleFiveNodesInitTransaction.h"

const BaseTransaction::TransactionType CycleFiveNodesInitTransaction::transactionType() const{
    return BaseTransaction::TransactionType::CycleFiveNodesInitTransaction;
}

TransactionResult::SharedConst CycleFiveNodesInitTransaction::runCollectDataAndSendMessagesStage() {
    vector<NodeUUID> firstLevelNodesNegativeBalance = mTrustLinesManager->firstLevelNeighborsWithNegativeBalance();
    vector<NodeUUID> firstLevelNodesPositiveBalance = mTrustLinesManager->firstLevelNeighborsWithPositiveBalance();
    vector<NodeUUID> path;
    path.push_back(mNodeUUID);
    TrustLineBalance zeroBalance = 0;
    for(const auto &value: firstLevelNodesNegativeBalance)
        sendMessage<CycleFiveNodesInBetweenMessage>(
            value,
            path
        );
    for(const auto &value: firstLevelNodesPositiveBalance)
            sendMessage<CycleFiveNodesInBetweenMessage>(
                value,
                path
            );
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultAwaikAfterMilliseconds(mWaitingForResponseTime);
}

CycleFiveNodesInitTransaction::CycleFiveNodesInitTransaction(
    const NodeUUID &nodeUUID, TransactionsScheduler *scheduler,
    TrustLinesManager *manager, Logger *logger)
    : CyclesBaseFiveSixNodesInitTransaction(BaseTransaction::TransactionType::CycleFiveNodesInitTransaction, nodeUUID, scheduler, manager, logger) {
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CycleFiveNodesInitTransaction::runParseMessageAndCreateCyclesStage() {

    TrustLineBalance zeroBalance = 0;
    CycleMap mCreditors;
    vector <NodeUUID> stepPath;
    TrustLineBalance creditorsStepFlow;
    for(const auto &mess: mContext){
        auto message = static_pointer_cast<CycleSixNodesBoundaryMessage>(mess);
        stepPath = message->Path();
//  It has to be exactly nodes count in path
        if (stepPath.size() != 2)
            continue;
        creditorsStepFlow = mTrustLinesManager->balance(stepPath[1]);
//  If it is Debtor branch - skip it
        if (creditorsStepFlow > zeroBalance)
            continue;
//  Check all Boundary Nodes and add it to map if all checks path
        for (auto &NodeUUIDAndBalance: message->BoundaryNodes()){
//  Prevent loop on cycles path
            if (NodeUUIDAndBalance.first == stepPath.front())
                continue;
//  NodeUUIDAndBalance.second - already minimum balance on creditors branch
//  For not tu use abc for every balance on debtors branch - just change sign of these balance
            mCreditors.insert(make_pair(
                NodeUUIDAndBalance.first,
                make_pair(stepPath, (-1) * NodeUUIDAndBalance.second)));

        }
    }

//    Create Cycles comparing BoundaryMessages data with debtors map
    ResultVector mCycles;
//     stepCyclePath;
    TrustLineBalance debtorsStepFlow;
    TrustLineBalance commonStepMaxFlow;
    for(const auto &mess: mContext) {
        auto message = static_pointer_cast<CycleSixNodesBoundaryMessage>(mess);
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
//  Find minMax flow between 3 value. 1 in map. 1 in boundaryNodes. 1 we get from creditor first node in path
                commonStepMaxFlow = min(min(s_it->second.second, debtorsStepFlow), NodeUUIDAndBalance.second);
                vector <NodeUUID> stepCyclePath = {stepPath[0],
                                                   stepPath[1],
                                                   stepPath[2],
                                                   NodeUUIDAndBalance.first,
                                                   s_it->second.first.back()};
                mCycles.push_back(make_pair(stepCyclePath, commonStepMaxFlow));
                stepCyclePath.clear();
            }
        }
    }
    mContext.clear();
//    Todo run cycles
    return finishTransaction();
}
#pragma clang diagnostic pop
