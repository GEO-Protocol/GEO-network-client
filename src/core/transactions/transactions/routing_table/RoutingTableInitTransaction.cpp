#include "RoutingTableInitTransaction.h"

RoutingTableInitTransaction::RoutingTableInitTransaction(
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *trustLinesManager,
    RoutingTableManager *routingTableManager,
    Logger &logger):

    BaseTransaction(
        BaseTransaction::TransactionType::RoutingTableInitTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mTrustLinesManager(trustLinesManager),
    mRoutingTableManager(routingTableManager),
    mLog(logger)
{}

TransactionResult::SharedConst RoutingTableInitTransaction::run()
{
    debug() << "run: stage: " << mStep;
    try {
        switch (mStep) {
            case Stages::CollectDataStage:
                return runCollectDataStage();

            case Stages::UpdateRoutingTableStage:
                return runUpdateRoutingTableStage();

            default:
                throw RuntimeError(
                    "RoutingTableInitTransaction::run(): "
                        "invalid transaction step.");
        }
    } catch(std::exception &e){
        error() << "Something happens wrong in method run(). "
                "Transaction will be dropped; " << e.what();
        return resultDone();
    }
}

TransactionResult::SharedConst RoutingTableInitTransaction::runCollectDataStage()
{
    auto neighbors = mTrustLinesManager->firstLevelNeighbors();
    for(const auto &kNeighborNode: neighbors){
        sendMessage<RoutingTableRequestMessage>(
            kNeighborNode,
            mEquivalent,
            currentNodeUUID());
    }
    mStep = Stages::UpdateRoutingTableStage;
    return resultAwakeAfterMilliseconds(
        mkWaitingForResponseTime);
}

TransactionResult::SharedConst RoutingTableInitTransaction::runUpdateRoutingTableStage()
{
    if (mContext.empty()){
        info() << "No responses from neighbors. RoutingTable will not be updated." << endl;
        return resultDone();
    }
    mRoutingTableManager->clearMap();

    for(auto &stepMessage: mContext){
        auto message = static_pointer_cast<RoutingTableResponseMessage>(stepMessage);
        if(!mTrustLinesManager->isNeighbor(message->senderUUID)){
            warning() << "Node " << message->senderUUID << " is not a neighbor";
            continue;
        }
        mRoutingTableManager->updateMapAddSeveralNeighbors(
            message->senderUUID,
            message->neighbors());
    }
    return resultDone();
}

const string RoutingTableInitTransaction::logHeader() const
{
    stringstream s;
    s << "[RoutingTableInitTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
