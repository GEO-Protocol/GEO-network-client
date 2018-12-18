#include "CyclesFourNodesInitTransaction.h"

CyclesFourNodesInitTransaction::CyclesFourNodesInitTransaction(
    const NodeUUID &nodeUUID,
    ContractorID contractorID,
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    RoutingTableManager *routingTable,
    CyclesManager *cyclesManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::Cycles_FourNodesInitTransaction,
        nodeUUID,
        equivalent,
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mCyclesManager(cyclesManager),
    mRoutingTable(routingTable),
    mContractorID(contractorID)
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
    debug() << "runCollectDataAndSendMessageStage to " << mContractorID;
    if (!mTrustLinesManager->trustLineIsActive(mContractorID)) {
        warning() << "TL with creditor is not active";
        return resultDone();
    }
    const auto kBalanceToContractor = mTrustLinesManager->balance(mContractorID);
    if (kBalanceToContractor == TrustLine::kZeroBalance()) {
        return resultDone();
    }

    vector<ContractorID> suitableNeighbors;
    if (kBalanceToContractor < TrustLine::kZeroBalance()) {
        suitableNeighbors = mTrustLinesManager->firstLevelNeighborsWithPositiveBalanceNew();
        mNegativeContractorBalance = true;
    } else {
        suitableNeighbors = mTrustLinesManager->firstLevelNeighborsWithNegativeBalanceNew();
        mNegativeContractorBalance = false;
    }

    auto creditorNeighbors = mRoutingTable->secondLevelContractorsForNode(
        mContractorID);

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
                    mContractorsManager->ownAddresses(),
                    currentTransactionUUID(),
                    mContractorsManager->contractorMainAddress(mContractorID),
                    commonNodes);
            } else {
                sendMessage<CyclesFourNodesPositiveBalanceRequestMessage>(
                    creditorNodeNeighbor,
                    mEquivalent,
                    mNodeUUID,
                    mContractorsManager->ownAddresses(),
                    currentTransactionUUID(),
                    mContractorsManager->contractorMainAddress(mContractorID),
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

#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    vector<Path::ConstShared> resultCycles;
#endif
    while (!mContext.empty()) {
        const auto kMessage = popNextMessage<CyclesFourNodesBalancesResponseMessage>();
        auto senderAddress = kMessage->senderAddresses.at(0);
        for (const auto &suitableNode : kMessage->suitableNodes()) {
            if (mContractorsManager->contractorIDByAddress(suitableNode) == ContractorsManager::kNotFoundContractorID) {
                warning() << "Suitable node " << suitableNode->fullAddress() << " is not a neighbor";
                continue;
            }
            vector<BaseAddress::Shared> stepPath = {
                suitableNode,
                senderAddress,
                mContractorsManager->contractorMainAddress(mContractorID)
            };

            if (!mNegativeContractorBalance) {
                reverse(
                    stepPath.begin(),
                    stepPath.end());
            }

            const auto cyclePath = make_shared<Path>(
                stepPath);
            mCyclesManager->addCycle(
                cyclePath);
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
            resultCycles.push_back(cyclePath);
#endif
        }
    }
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    debug() << "ResultCyclesCount " << resultCycles.size();
    for (auto &cyclePath: resultCycles) {
        debug() << "CyclePath " << cyclePath->toString();
    }
#endif
    mCyclesManager->closeOneCycle();
    return resultDone();
}

vector<BaseAddress::Shared> CyclesFourNodesInitTransaction::getCommonNodes(
    BaseAddress::Shared creditorNeighborNode,
    vector<ContractorID> currentNodeSuitableNeighbors)
{
    vector<BaseAddress::Shared> result;
    for (const auto &suitableNeighbor : currentNodeSuitableNeighbors) {
        for (const auto &secondLevelNeighbor : mRoutingTable->secondLevelContractorsForNode(suitableNeighbor)) {
            if (secondLevelNeighbor == creditorNeighborNode) {
                result.push_back(
                    mContractorsManager->contractorMainAddress(
                        suitableNeighbor));
            }
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