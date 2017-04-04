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
    TrustLineBalance stepFlow;
    for(const auto &mess: mContext){
        auto message = static_pointer_cast<CycleSixNodesBoundaryMessage>(mess);
// iF max flow less than zero than add this message to map
        stepFlow = mTrustLinesManager->balance(message->Path()[1]);
        if (stepFlow < zeroBalance){
            stepPath = message->Path();
            for (auto &value: message->BoundaryNodes()){
                mCreditors.insert(make_pair(
                    value.first,
                    make_pair(stepPath, min(stepFlow, value.second))));
            }
        }
    }
//    Create Cycles comparing BoundaryMessages data with debtors map
    ResultVector mCycles;
    vector <NodeUUID> stepCyclePath;
    for(const auto &mess: mContext){
        auto message = static_pointer_cast<CycleSixNodesBoundaryMessage>(mess);
        stepFlow = mTrustLinesManager->balance(message->Path()[1]);
        if (stepFlow > zeroBalance){
            stepPath = message->Path();
            for (auto &value: message->BoundaryNodes()){
                mapIter m_it, s_it;
                pair<mapIter, mapIter> keyRange = mCreditors.equal_range(value.first);
                for (s_it = keyRange.first;  s_it != keyRange.second;  ++s_it){
                    stepCyclePath = stepPath;
                    stepCyclePath.push_back(value.first);
                    for (uint8_t  i=s_it->second.first.size() -1 ; i>0; i--)
                        stepCyclePath.push_back(s_it->second.first[i]);
                    mCycles.push_back(make_pair(stepCyclePath, min(stepFlow, value.second)));
                    stepCyclePath.clear();
                }
            }
        }
    }
    mContext.clear();
//    Todo run cycles
    return finishTransaction();
}
#pragma clang diagnostic pop
