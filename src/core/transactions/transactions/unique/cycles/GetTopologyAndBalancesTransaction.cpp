#include "GetTopologyAndBalancesTransaction.h"
#include "../../base/UniqueTransaction.cpp"

GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(const TransactionType type,
                                                                     const NodeUUID &nodeUUID,
                                                                     TransactionsScheduler *scheduler,
                                                                     TrustLinesManager *manager,
                                                                     Logger *logger
)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mTrustLinesManager(manager),
          mlogger(logger)
{
};

GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(const TransactionType type,
                                                                     const NodeUUID &nodeUUID,
                                                                     InBetweenNodeTopologyMessage::Shared message,
                                                                     TransactionsScheduler *scheduler,
                                                                     TrustLinesManager *manager, Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mInBetweeenMessage(message),
          mTrustLinesManager(manager),
          mlogger(logger){
}

GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(TransactionsScheduler *scheduler)
        : UniqueTransaction(BaseTransaction::TransactionType::GetTopologyAndBalancesTransaction, scheduler) {

}

TransactionResult::SharedConst GetTopologyAndBalancesTransaction::run() {
//   It is initiator node and we have already have some topology
    if (mWaitingFowAnswer) {
        if (mContext.size() > 0) {
            createCyclesFromResponses();
        } else {
            auto state = TransactionState::exit();
            return make_shared<TransactionResult>(state);
            }
    } else if (mInBetweeenMessage != nullptr ) {
 //  It is not initiator node. So we will decide to send messages to next level nodes, or to send it back to initiator
        vector<pair<NodeUUID, TrustLineBalance>> firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(
                mInBetweeenMessage->maxFlow());
        vector<NodeUUID> path = mInBetweeenMessage->Path();
        path.push_back(mNodeUUID);
        if (path.size() > mInBetweeenMessage->maxDepth()) {
//      Send message back to node initiator
            sendMessage<BoundaryNodeTopologyMessage>(
                    mInBetweeenMessage->Path().front(),
                    cycleType(),
                    mInBetweeenMessage->maxFlow(),
                    mInBetweeenMessage->maxDepth(),
                    path,
                    firstLevelNodes
            );
            auto state = TransactionState::exit();
            return make_shared<TransactionResult>(state);
        } else {
//      Send it to next level nodes
            for(const auto &value: firstLevelNodes){
                sendMessage<InBetweenNodeTopologyMessage>(
                        value.first,
                        cycleType(),
                        min(mInBetweeenMessage->maxFlow(), value.second),
                        mInBetweeenMessage->maxDepth(),
                        path
                );
            }
            auto state = TransactionState::exit();
            return make_shared<TransactionResult>(state);
        }
    }
//        It is initiator node and we have no topology yet
    else {
        mWaitingFowAnswer = true;
        sendFirstLevelNodeMessage();
        auto state = TransactionState::awakeAfterMilliseconds(mWaitingForResponseTime);
        return make_shared<TransactionResult>(state);
    }
    return TransactionResult::SharedConst();
}

void GetTopologyAndBalancesTransaction::createCyclesFromResponses() {
    TrustLineBalance zeroBalance = 0;
    vector <NodeUUID> stepPath;
    TrustLineBalance stepFlow;
    vector <NodeUUID> stepCyclePath;
    for(const auto &mess: mContext){
        auto message = static_pointer_cast<BoundaryNodeTopologyMessage>(mess);
// iF max flow less than zero than add this message to map
        stepFlow = message->maxFlow();
        if (stepFlow < zeroBalance){
            stepPath = message->Path();
            for (auto &value: message->BoundaryNodes()){
                mDebtors.insert(make_pair(
                        value.first,
                        make_pair(stepPath, min(stepFlow, value.second))));
            }
        }
    }

//    Create Cycles comparing BoundaryMessages data with debtors map
    for(const auto &mess: mContext){
        auto message = static_pointer_cast<BoundaryNodeTopologyMessage>(mess);
        stepFlow = message->maxFlow();
        if (stepFlow > zeroBalance){
            stepPath = message->Path();
            for (auto &value: message->BoundaryNodes()){
                mapIter m_it, s_it;
                pair<mapIter, mapIter> keyRange = mDebtors.equal_range(value.first);
                for (s_it = keyRange.first;  s_it != keyRange.second;  ++s_it){
                    stepCyclePath = stepPath;
                    stepCyclePath.push_back(value.first);
                    for (unsigned long i=s_it->second.first.size() -1 ; i>0; i--)
                        stepCyclePath.push_back(s_it->second.first[i]);
                    mCycles.push_back(make_pair(stepCyclePath, min(stepFlow, value.second)));
                    stepCyclePath.clear();
                }
            }
        }
    }
    mContext.clear();
}

