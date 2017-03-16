#include "FiveNodesTopologyTransaction.h"

const BaseTransaction::TransactionType FiveNodesTopologyTransaction::transactionType() const{
    return BaseTransaction::TransactionType::FiveNodesTopologyTransaction;
}

void FiveNodesTopologyTransaction::sendFirstLevelNodeMessage() {
    vector<pair<NodeUUID, TrustLineBalance>> firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(0);
    vector<NodeUUID> path;
    path.push_back(mNodeUUID);
    TrustLineBalance zeroBalance = 0;
    for(const auto &value: firstLevelNodes){
        if(value.second  < zeroBalance){
            sendMessage<InBetweenNodeTopologyMessage>(
                    value.first,
                    cycleType(),
                    value.second,
                    mMaxDepthCreditors,
                    path
            );
        }
        else {
            sendMessage<InBetweenNodeTopologyMessage>(
                    value.first,
                    cycleType(),
                    value.second,
                    mMaxDepthDebtors,
                    path
            );
        }
    }
}

FiveNodesTopologyTransaction::FiveNodesTopologyTransaction(const BaseTransaction::TransactionType type,
                                                           const NodeUUID &nodeUUID, TransactionsScheduler *scheduler,
                                                           TrustLinesManager *manager, Logger *logger)
        : GetTopologyAndBalancesTransaction(type, nodeUUID, scheduler, manager, logger) {

}

FiveNodesTopologyTransaction::FiveNodesTopologyTransaction(const BaseTransaction::TransactionType type,
                                                           const NodeUUID &nodeUUID,
                                                           InBetweenNodeTopologyMessage::Shared message,
                                                           TransactionsScheduler *scheduler, TrustLinesManager *manager,
                                                           Logger *logger) : GetTopologyAndBalancesTransaction(type,
                                                                                                               nodeUUID,
                                                                                                               message,
                                                                                                               scheduler,
                                                                                                               manager,
                                                                                                               logger) {

}

InBetweenNodeTopologyMessage::CycleTypeID FiveNodesTopologyTransaction::cycleType() {
    return InBetweenNodeTopologyMessage::CycleTypeID::CycleForFiveNodes;
};