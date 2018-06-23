#include "CyclesFourNodesInitTransaction.h"

CyclesFourNodesInitTransaction::CyclesFourNodesInitTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    RoutingTableManager *routingTable,
    CyclesManager *cyclesManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesInitTransaction,
        nodeUUID,
        equivalent,
        logger),
    mTrustLinesManager(manager),
    mCyclesManager(cyclesManager),
    mRoutingTable(routingTable),
    mContractorUUID(contractorUUID)
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
    debug() << "runCollectDataAndSendMessageStage; creditor is " << mContractorUUID;
    if (!mTrustLinesManager->trustLineIsActive(mContractorUUID)) {
        warning() << "TL with creditor is not active";
        return resultDone();
    }
    const auto kBalanceToContractor = mTrustLinesManager->balance(mContractorUUID);
    if (kBalanceToContractor == TrustLine::kZeroBalance()) {
        return resultDone();
    }

    vector<NodeUUID> suitableNeighbors;
    if (kBalanceToContractor < TrustLine::kZeroBalance()) {
        suitableNeighbors = mTrustLinesManager->firstLevelNeighborsWithPositiveBalance();
        mNegativeContractorBalance = true;
    } else {
        suitableNeighbors = mTrustLinesManager->firstLevelNeighborsWithNegativeBalance();
        mNegativeContractorBalance = false;
    }

    auto creditorNeighbors = mRoutingTable->secondLevelContractorsForNode(
        mContractorUUID);

    bool sendMessageToAtLeastOneNode = false;
    for (const auto &creditorNodeNeighbor : creditorNeighbors) {
        auto commonNodes = getCommonNodes(
            creditorNodeNeighbor,
            suitableNeighbors);
        if (!commonNodes.empty()) {
            if (mNegativeContractorBalance) {
                sendMessage<CyclesFourNodesNegativeBalanceRequestMessage>(
                    creditorNodeNeighbor,
                    mEquivalent,
                    mNodeUUID,
                    currentTransactionUUID(),
                    mContractorUUID,
                    commonNodes);
            } else {
                sendMessage<CyclesFourNodesPositiveBalanceRequestMessage>(
                    creditorNodeNeighbor,
                    mEquivalent,
                    mNodeUUID,
                    currentTransactionUUID(),
                    mContractorUUID,
                    commonNodes);
            }
            sendMessageToAtLeastOneNode = true;
        }
    }

    if (!sendMessageToAtLeastOneNode) {
        info() << "There are no suitable nodes for building cycles";
        return resultDone();
    }

    mStep = Stages::ParseMessageAndCreateCycles;
    return resultWaitForMessageTypesAndAwakeAfterMilliseconds(
        {Message::MessageType::Cycles_FourNodesBalancesResponse},
        mkWaitingForResponseTime);
}

TransactionResult::SharedConst CyclesFourNodesInitTransaction::runParseMessageAndCreateCyclesStage()
{
    debug() << "runParseMessageAndCreateCyclesStage";
    if (mContext.empty()) {
        info() << "No responses messages are present. Can't create cycles paths;";
        return resultDone();
    }

    while (!mContext.empty()) {
        const auto kMessage = popNextMessage<CyclesFourNodesBalancesResponseMessage>();
        auto sender = kMessage->senderUUID;
        for (const auto &suitableNode : kMessage->suitableNodes()) {
            vector<NodeUUID> stepPath = {
                suitableNode,
                sender,
                mContractorUUID
            };

            if (!mNegativeContractorBalance) {
                reverse(
                    stepPath.begin(),
                    stepPath.end());
            }

            const auto cyclePath = make_shared<Path>(
                mNodeUUID,
                mNodeUUID,
                stepPath);
            mCyclesManager->addCycle(
                cyclePath);
        }
    }
    mCyclesManager->closeOneCycle();
    return resultDone();
}

vector<NodeUUID> CyclesFourNodesInitTransaction::getCommonNodes(
    const NodeUUID &creditorNeighborNode,
    vector<NodeUUID> currentNodeSuitableNeighbors)
{
    vector<NodeUUID> result;
    for (const auto &suitableNeighbor : currentNodeSuitableNeighbors) {
        if (mRoutingTable->secondLevelContractorsForNode(suitableNeighbor).count(creditorNeighborNode) != 0) {
            result.push_back(suitableNeighbor);
        }
    }
    return result;
}

const string CyclesFourNodesInitTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFourNodesInitTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}