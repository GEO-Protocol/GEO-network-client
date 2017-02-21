#include "GetTopologyAndBalancesTransaction.h"
#include "../../base/UniqueTransaction.cpp"

GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(TransactionType type,
                                                                     NodeUUID &nodeUUID,
                                                                     TransactionsScheduler *scheduler,
                                                                     TrustLinesManager *manager,
                                                                     Logger *logger
)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mTrustLinesManager(manager),
          mlogger(logger)
{

};

GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(TransactionType type,
                                                                     NodeUUID &nodeUUID,
                                                                     InBetweenNodeTopologyMessage::Shared message,
                                                                     TransactionsScheduler *scheduler,
                                                                     TrustLinesManager *manager, Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mInBetweeenMessage(message),
          mTrustLinesManager(manager),
          mlogger(logger){
}

GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(TransactionType type,
                                                                     NodeUUID &nodeUUID,
                                                                     BoundaryNodeTopolodyMessage::Shared message,
                                                                     TransactionsScheduler *scheduler,
                                                                     TrustLinesManager *manager, Logger *logger)
        : UniqueTransaction(type, nodeUUID, scheduler),
          mBoundaryMessage(message),
          mTrustLinesManager(manager),
          mlogger(logger){
}

GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(TransactionsScheduler *scheduler)
        : UniqueTransaction(scheduler) {

}

TransactionResult::SharedConst GetTopologyAndBalancesTransaction::run() {

    if (mInBetweeenMessage != nullptr ){
        vector<pair<NodeUUID, TrustLineBalance>> firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(
                mInBetweeenMessage->getMaxFlow());
        vector<NodeUUID> path = mInBetweeenMessage->getPath();
        path.push_back(mNodeUUID);
            for(const auto &value: firstLevelNodes){
                addMessage(
                        Message::Shared(new InBetweenNodeTopologyMessage(
                                value.second,
                                mMax_depth,
                                path
                        )),
                        value.first
                );
            }
    } else if(mBoundaryMessage != nullptr){
        NodeUUID initiatorNode = mBoundaryMessage->getPath().front();
    }
    else {
        vector<pair<NodeUUID, TrustLineBalance>> firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(0);
        vector<NodeUUID> path;
        path.push_back(mNodeUUID);
        for(const auto &value: firstLevelNodes){
            addMessage(
                    Message::Shared(new InBetweenNodeTopologyMessage(
                            value.second,
                            mMax_depth,
                            path
                    )),
                    value.first
            );
        }
    }

    return TransactionResult::SharedConst();
}
pair<BytesShared, size_t> GetTopologyAndBalancesTransaction::serializeToBytes() const {
    throw ValueError("Not implemented");
}

void GetTopologyAndBalancesTransaction::deserializeFromBytes(BytesShared buffer) {
    throw ValueError("Not implemented");
}

