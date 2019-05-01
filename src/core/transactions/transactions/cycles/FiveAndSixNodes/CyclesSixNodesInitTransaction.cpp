#include "CyclesSixNodesInitTransaction.h"

CyclesSixNodesInitTransaction::CyclesSixNodesInitTransaction(
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    CyclesManager *cyclesManager,
    TailManager *tailManager,
    Logger &logger) :
    CyclesBaseFiveSixNodesInitTransaction(
        BaseTransaction::Cycles_SixNodesInitTransaction,
        equivalent,
        contractorsManager,
        trustLinesManager,
        cyclesManager,
        tailManager,
        logger)
{}

TransactionResult::SharedConst CyclesSixNodesInitTransaction::runCollectDataAndSendMessagesStage()
{
    debug() << "runCollectDataAndSendMessagesStage";
    vector<BaseAddress::Shared> path;
    path.push_back(
        mContractorsManager->selfContractor()->mainAddress());
    for(const auto &neighborID: mTrustLinesManager->firstLevelNeighborsWithNoneZeroBalance()){
        sendMessage<CyclesSixNodesInBetweenMessage>(
            neighborID,
            mEquivalent,
            mContractorsManager->idOnContractorSide(neighborID),
            path);
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
        debug() << "Send message to neighbor " << neighborID;
#endif
    }
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultAwakeAfterMilliseconds(mkWaitingForResponseTime);
}

TransactionResult::SharedConst CyclesSixNodesInitTransaction::runParseMessageAndCreateCyclesStage()
{
    debug() << "runParseMessageAndCreateCyclesStage";
    auto &mContext = mTailManager->getCyclesSixTail();
    if (mContext.empty()) {
        info() << "No responses messages are present. Can't create cycles paths;";
        return resultDone();
    }
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    debug() << "Context size: " << mContext.size();
#endif
    CycleMap creditors;
    CycleMap debtors;
    TrustLineBalance creditorsStepFlow;
    while (!mContext.empty()) {
        const auto message = popNextMessage<CyclesSixNodesBoundaryMessage>(mContext);
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
        debug() << "Receive message from " << message->senderIncomingIP();
        debug() << "Path:";
        for (const auto &node : message->path()) {
            debug() << node->fullAddress();
        }
        debug() << "Boundary nodes:";
        for (const auto &node : message->boundaryNodes()) {
            debug() << node->fullAddress();
        }
#endif
        if (message->equivalent() != mEquivalent) {
            warning() << "Message belongs to equivalent " << message->equivalent();
            continue;
        }
        auto stepPath = message->path();
        //  It has to be exactly nodes count in path
        if (stepPath.size() != 3) {
            warning() << "Received message contains " << stepPath.size() << " nodes";
            continue;
        }
        if (stepPath.front() != mContractorsManager->selfContractor()->mainAddress()) {
            warning() << "Received message was initiate by other node " << stepPath.front()->fullAddress();
            continue;
        }
        auto contractorID = mContractorsManager->contractorIDByAddress(stepPath[1]);
        if (contractorID == ContractorsManager::kNotFoundContractorID) {
            warning() << "There is no contractor with address " << stepPath[1]->fullAddress();
            continue;
        }

        if (mTrustLinesManager->balance(contractorID) > TrustLine::kZeroBalance()) {
            //  If it is Debtor branch
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
            debug() << "Debtor message";
#endif
            //  Check all Boundary Nodes and add it to map if all checks path
            for (auto &nodeAddress: message->boundaryNodes()) {
                //  Prevent loop on cycles path
                if (nodeAddress == stepPath.front()) {
                    continue;
                }
                debtors.insert(
                    make_pair(
                        nodeAddress->fullAddress(),
                        stepPath));
            }
        } else if (mTrustLinesManager->balance(contractorID) < TrustLine::kZeroBalance()) {
            //  If it is Creditor branch
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
            debug() << "Creditor message";
#endif
            //  Check all Boundary Nodes and add it to map if all checks path
            for (auto &nodeAddress: message->boundaryNodes()) {
                //  Prevent loop on cycles path
                if (nodeAddress == stepPath.front()) {
                    continue;
                }
                creditors.insert(
                    make_pair(
                        nodeAddress->fullAddress(),
                        stepPath));
            }
        } else {
            // zero balance - skip it
            continue;
        }
    }

    //    Create Cycles comparing data from creditors map with debtors map
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    vector<Path::ConstShared> resultCycles;
#endif
    for(const auto &debtorPackage : debtors) {
        auto nodeAddressAndPathRange = creditors.equal_range(debtorPackage.first);
        for (auto nodeAddressAndPathIt = nodeAddressAndPathRange.first;
            nodeAddressAndPathIt != nodeAddressAndPathRange.second; ++nodeAddressAndPathIt) {
            if ((nodeAddressAndPathIt->second[2] == debtorPackage.second[1]) or
                (nodeAddressAndPathIt->second[1] == debtorPackage.second[2]) or
                (nodeAddressAndPathIt->second[1] == debtorPackage.second[1]) or
                (nodeAddressAndPathIt->second[2] == debtorPackage.second[2])) {
                continue;
            }
            //  Find minMax flow between 3 value. 1 in map. 1 in boundaryNodes. 1 we get from creditor first node in path
            vector<BaseAddress::Shared> stepCyclePath = {
                    debtorPackage.second[1],
                    debtorPackage.second[2],
                    // todo adapt for all types of address
                    make_shared<IPv4WithPortAddress>(
                        debtorPackage.first),
                    (nodeAddressAndPathIt->second)[2],
                    (nodeAddressAndPathIt->second)[1]};
            const auto cyclePath = make_shared<Path>(
                stepCyclePath);
            mCyclesManager->addCycle(
               cyclePath);
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
            resultCycles.push_back(cyclePath);
#endif
        }
    }
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    debug() << "ResultCyclesCount " << resultCycles.size();
    for (auto &cyclePath: resultCycles){
        debug() << "CyclePath " << cyclePath->toString();
    }
#endif
    mCyclesManager->closeOneCycle();
    return resultDone();
}

const string CyclesSixNodesInitTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesSixNodesInitTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}