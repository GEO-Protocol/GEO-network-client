#include "SixNodesTopologyTransaction.h"

const BaseTransaction::TransactionType SixNodesTopologyTransaction::transactionType() const{
    return BaseTransaction::TransactionType::SixNodesTopologyTransaction;
}

void SixNodesTopologyTransaction::sendFirstLevelNodeMessage() {
        vector<pair<NodeUUID, TrustLineBalance>> firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(0);
        vector<NodeUUID> path;
        path.push_back(mNodeUUID);
        for(const auto &value: firstLevelNodes){
            sendMessage<InBetweenNodeTopologyMessage>(
                    value.first,
                    cycleType(),
                    value.second,
                    mMaxDepth,
                    path
            );
        }
}

SixNodesTopologyTransaction::SixNodesTopologyTransaction(const BaseTransaction::TransactionType type,
                                                         const NodeUUID &nodeUUID, TransactionsScheduler *scheduler,
                                                         TrustLinesManager *manager, Logger *logger)
        : GetTopologyAndBalancesTransaction(type, nodeUUID, scheduler, manager, logger) {

}

SixNodesTopologyTransaction::SixNodesTopologyTransaction(const BaseTransaction::TransactionType type,
                                                         const NodeUUID &nodeUUID,
                                                         InBetweenNodeTopologyMessage::Shared message,
                                                         TransactionsScheduler *scheduler, TrustLinesManager *manager,
                                                         Logger *logger) : GetTopologyAndBalancesTransaction(type,
                                                                                                             nodeUUID,
                                                                                                             message,
                                                                                                             scheduler,
                                                                                                             manager,
                                                                                                             logger) {

};

InBetweenNodeTopologyMessage::CycleTypeID SixNodesTopologyTransaction::cycleType() {
    return InBetweenNodeTopologyMessage::CycleTypeID::CycleForSixNodes;
}