#include "CyclesFiveNodesInitTransaction.h"

const BaseTransaction::TransactionType CyclesFiveNodesInitTransaction::transactionType() const{
    return BaseTransaction::TransactionType::CyclesFiveNodesInitTransaction;
}


TransactionResult::SharedConst CyclesFiveNodesInitTransaction::runCollectDataAndSendMessagesStage() {
    vector<NodeUUID> firstLevelNodesNegativeBalance = mTrustLinesManager->firstLevelNeighborsWithNegativeBalance();
    vector<NodeUUID> firstLevelNodesPositiveBalance = mTrustLinesManager->firstLevelNeighborsWithPositiveBalance();
    vector<NodeUUID> path;
    path.push_back(mNodeUUID);
    TrustLineBalance zeroBalance = 0;
    for(const auto &value: firstLevelNodesNegativeBalance)
        sendMessage<InBetweenNodeTopologyMessage>(
            value,
            cycleType(),
            mMaxDepthCreditors,
            path
        );

    for(const auto &value: firstLevelNodesPositiveBalance)
            sendMessage<InBetweenNodeTopologyMessage>(
                value,
                cycleType(),
                mMaxDepthDebtors,
                path
            );
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultAwaikAfterMilliseconds(mWaitingForResponseTime);
}

CyclesFiveNodesInitTransaction::CyclesFiveNodesInitTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID, TransactionsScheduler *scheduler,
    TrustLinesManager *manager, Logger *logger)
    : CyclesBaseFiveSixNodesInitTransaction(type, nodeUUID, scheduler, manager, logger) {

}

InBetweenNodeTopologyMessage::CycleTypeID CyclesFiveNodesInitTransaction::cycleType() {
    return InBetweenNodeTopologyMessage::CycleTypeID::CycleForFiveNodes;
};