#include "CyclesFourNodesInitTransaction.h"

CyclesFourNodesInitTransaction::CyclesFourNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &creditorContractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    RoutingTableManager *routingTable,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesInitTransaction,
        nodeUUID,
        equivalent,
        logger),
    mTrustLinesManager(manager),
    mCyclesManager(cyclesManager),
    mRoutingTable(routingTable),
    mStorageHandler(storageHandler),
    mCreditorContractorUUID(creditorContractorUUID)
{}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::run()
{
    switch (mStep) {
        case Stages::CollectDataAndSendMessage:
            return runCollectDataAndSendMessageStage();

        case Stages::ParseMessageAndCreateCycles:
            return runParseMessageAndCreateCyclesStage();

        default:
            throw RuntimeError(
                "CyclesFourNodesInitTransaction::run(): "
                "Invalid transaction step.");
    }
}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::runCollectDataAndSendMessageStage()
{
    debug() << "runCollectDataAndSendMessageStage; Receiver is " << mCreditorContractorUUID;

    auto neighborsDebtors = mTrustLinesManager->firstLevelNeighborsWithPositiveBalance();
    for(const auto &kDebtorNode:neighborsDebtors){
        auto commonNodes = calculateCommonNodes(kDebtorNode, mCreditorContractorUUID);
        if(commonNodes.size() == 0 )
            continue;
        for(const auto kCommonNode: commonNodes){
            mWaitingResponses[kCommonNode] = kDebtorNode;
            sendMessage<CyclesFourNodesBalancesRequestMessage>(
                kCommonNode,
                mEquivalent,
                mNodeUUID,
                currentTransactionUUID(),
                kDebtorNode,
                mCreditorContractorUUID);
        }
    }

    mStep = Stages::ParseMessageAndCreateCycles;
    return resultWaitForMessageTypesAndAwakeAfterMilliseconds(
        {Message::MessageType::Cycles_FourNodesBalancesResponse},
        mkWaitingForResponseTime);
}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage()
{
    debug() << "runParseMessageAndCreateCyclesStage";
    if (mContext.size() == 0) {
        info() << "No responses messages are present. Can't create cycles paths;";
        return resultDone();
    }

    for(auto message:mContext){
        const auto kMessage = static_pointer_cast<CyclesFourNodesBalancesResponseMessage>(message);
        auto sender = kMessage->senderUUID;
        vector<NodeUUID> stepPath = {
            mWaitingResponses[sender],
            sender,
            mCreditorContractorUUID
        };

        const auto cyclePath = make_shared<Path>(
            mNodeUUID,
            mNodeUUID,
            stepPath);
        mCyclesManager->addCycle(
            cyclePath);
    }
    mCyclesManager->closeOneCycle();
    return resultDone();
}

const string CyclesFourNodesInitTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFourNodesInitTransactionTA: " << currentTransactionUUID() << "] ";
    return s.str();
}

vector<NodeUUID> CyclesFourNodesInitTransaction::calculateCommonNodes(
    const NodeUUID &firstNode,
    const NodeUUID &secondNode)
{
    auto secondNodeNeighbors = mRoutingTable->secondLevelContractorsForNode(secondNode);
    auto firstNodeNeighbors = mRoutingTable->secondLevelContractorsForNode(firstNode);
    vector<NodeUUID> commonNeighbors;

    set_intersection(
        secondNodeNeighbors.begin(),
        secondNodeNeighbors.end(),
        firstNodeNeighbors.begin(),
        firstNodeNeighbors.end(),
        std::inserter(
            commonNeighbors,
            commonNeighbors.begin()));
    return commonNeighbors;
}