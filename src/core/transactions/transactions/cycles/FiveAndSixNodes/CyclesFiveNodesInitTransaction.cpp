#include "CyclesFiveNodesInitTransaction.h"

CyclesFiveNodesInitTransaction::CyclesFiveNodesInitTransaction(
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    CyclesManager *cyclesManager,
    TailManager *tailManager,
    Logger &logger) :
    CyclesBaseFiveSixNodesInitTransaction(
        BaseTransaction::Cycles_FiveNodesInitTransaction,
        equivalent,
        contractorsManager,
        trustLinesManager,
        cyclesManager,
        logger),
    mTailManager(tailManager)
{}

TransactionResult::SharedConst CyclesFiveNodesInitTransaction::runCollectDataAndSendMessagesStage()
{
    info() << "runCollectDataAndSendMessagesStage";
    debug() << "Cycles five tails count " << mTailManager->getCyclesFiveTail().size();
    debug() << "Cycles six tails count " << mTailManager->getCyclesSixTail().size();
    debug() << "Topology tails count " << mTailManager->getFlowTail().size();
    vector<BaseAddress::Shared> path;
    path.push_back(
        mContractorsManager->selfContractor()->mainAddress());
    TrustLineBalance zeroBalance = 0;
    for(const auto &neighborID: mTrustLinesManager->firstLevelNeighborsWithNegativeBalance()) {
        sendMessage<CyclesFiveNodesInBetweenMessage>(
            neighborID,
            mEquivalent,
            mContractorsManager->idOnContractorSide(neighborID),
            path);
    }
    for(const auto &neighborID: mTrustLinesManager->firstLevelNeighborsWithPositiveBalance()) {
        sendMessage<CyclesFiveNodesInBetweenMessage>(
            neighborID,
            mEquivalent,
            mContractorsManager->idOnContractorSide(neighborID),
            path);
    }
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultAwakeAfterMilliseconds(mkWaitingForResponseTime);
}

TransactionResult::SharedConst CyclesFiveNodesInitTransaction::runParseMessageAndCreateCyclesStage()
{
    info() << "runParseMessageAndCreateCyclesStage";
    auto &mContext = mTailManager->getCyclesFiveTail();
    if (mContext.empty()) {
        info() << "No responses messages are present. Can't create cycles paths";
        return resultDone();
    }
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    debug() << "Context size: " << mContext.size();
#endif
    CycleMap creditors;
    CycleMap debtors;
    while (!mContext.empty()) {
        const auto message = popNextMessage<CyclesFiveNodesBoundaryMessage>(mContext);
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
        debug() << "Receive message from " << message->senderIncomingIP();
        debug() << "Message path";
        for (const auto &node : message->path()) {
            debug() << "\t" << node->fullAddress();
        }
        debug() << "boundary nodes";
        for (const auto &node : message->boundaryNodes()) {
            debug() << "\t" << node->fullAddress();
        }
#endif
        if (message->equivalent() != mEquivalent) {
            warning() << "Message belongs to equivalent " << message->equivalent();
            continue;
        }
        auto stepPath = message->path();
        //  It has to be exactly nodes count in path
        if (stepPath.size() < 2) {
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

        if (mTrustLinesManager->balance(contractorID) < TrustLine::kZeroBalance()) {
            //  If it is Debtor branch
            if (stepPath.size() != 3) {
                warning() << "Received message contains " << stepPath.size() << " nodes";
                continue;
            }
            //  It has to be exactly nodes count in path
            for (const auto &nodeAddress: message->boundaryNodes()) {
                //  Prevent loop on cycles path
                if (nodeAddress == stepPath.front()) {
                    continue;
                }
                //  For not to use abc for every balance on debtors branch - just change sign of these balance
                debtors.insert(
                    make_pair(
                        nodeAddress->fullAddress(),
                        stepPath));
            }
        } else if (mTrustLinesManager->balance(contractorID) > TrustLine::kZeroBalance()) {
            //  If it is Creditor branch
            if (stepPath.size() != 2) {
                warning() << "Received message contains " << stepPath.size() << " nodes";
                continue;
            }
            //  Check all Boundary Nodes and add it to map if all checks path
            for (const auto &nodeAddress: message->boundaryNodes()) {
                //  Prevent loop on cycles path
                if (nodeAddress == stepPath.front()) {
                    continue;
                }
                //  For not to use abc for every balance on debtors branch - just change sign of these balance
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
            if ((nodeAddressAndPathIt->second.back() == debtorPackage.second[1]) or
                (nodeAddressAndPathIt->second.back() == debtorPackage.second[2])) {
                continue;
            }

            vector<BaseAddress::Shared> stepCyclePath = {
                    nodeAddressAndPathIt->second.back(),
                    // todo adapt for all types of address
                    make_shared<IPv4WithPortAddress>(
                        debtorPackage.first),
                    debtorPackage.second[2],
                    debtorPackage.second[1]};

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

const string CyclesFiveNodesInitTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFiveNodesInitTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
