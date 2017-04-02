#include "CyclesSixNodesInitTransaction.h"

const BaseTransaction::TransactionType CyclesSixNodesInitTransaction::transactionType() const{
    return BaseTransaction::TransactionType::CyclesSixNodesInitTransaction;
}


TransactionResult::SharedConst CyclesSixNodesInitTransaction::runCollectDataAndSendMessagesStage() {
    auto firstLevelNodes = mTrustLinesManager->firstLevelNeighborsWithNoneZeroBalance();
    vector<NodeUUID> path;
    path.push_back(mNodeUUID);
    for(const auto &value: firstLevelNodes){
        sendMessage<InBetweenNodeTopologyMessage>(
            value,
            cycleType(),
            mMaxDepth,
            path
        );
    }
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultAwaikAfterMilliseconds(mWaitingForResponseTime);
}

CyclesSixNodesInitTransaction::CyclesSixNodesInitTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID, TransactionsScheduler *scheduler,
    TrustLinesManager *manager, Logger *logger)
    : CyclesBaseFiveSixNodesInitTransaction(type, nodeUUID, scheduler, manager, logger) {

}

InBetweenNodeTopologyMessage::CycleTypeID CyclesSixNodesInitTransaction::cycleType() {
    return InBetweenNodeTopologyMessage::CycleTypeID::CycleForSixNodes;
};