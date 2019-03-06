#include "CyclesFiveNodesInitTransaction.h"

CyclesFiveNodesInitTransaction::CyclesFiveNodesInitTransaction(
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    CyclesManager *cyclesManager,
    Logger &logger) :
    CyclesBaseFiveSixNodesInitTransaction(
        BaseTransaction::Cycles_FiveNodesInitTransaction,
        equivalent,
        contractorsManager,
        trustLinesManager,
        cyclesManager,
        logger)
{}

TransactionResult::SharedConst CyclesFiveNodesInitTransaction::runCollectDataAndSendMessagesStage()
{
    debug() << "runCollectDataAndSendMessagesStage";
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
    debug() << "runParseMessageAndCreateCyclesStage";
    if (mContext.empty()) {
        info() << "No responses messages are present. Can't create cycles paths";
        return resultDone();
    }
    CycleMap creditors;
    TrustLineBalance creditorsStepFlow;
    for (const auto &mess: mContext) {
        auto message = static_pointer_cast<CyclesFiveNodesBoundaryMessage>(mess);
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
        creditorsStepFlow = mTrustLinesManager->balance(contractorID);
        //  If it is Debtor branch - skip it
        if (creditorsStepFlow <= TrustLine::kZeroBalance()) {
            continue;
        }
        if (stepPath.size() != 2) {
            warning() << "Received message contains " << stepPath.size() << " creditor nodes";
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
    }

    //    Create Cycles comparing BoundaryMessages data with debtors map
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    vector<Path::ConstShared> resultCycles;
#endif
    TrustLineBalance debtorsStepFlow;
    TrustLineBalance commonStepMaxFlow;
    for(const auto &mess: mContext) {
        auto message = static_pointer_cast<CyclesFiveNodesBoundaryMessage>(mess);
        auto stepPathDebtors = message->path();
        if (stepPathDebtors.size() < 2) {
            warning() << "Received message contains " << stepPathDebtors.size() << " nodes";
            continue;
        }
        if (stepPathDebtors.front() != mContractorsManager->selfContractor()->mainAddress()) {
            warning() << "Received message was initiate by other node " << stepPathDebtors.front()->fullAddress();
            continue;
        }
        auto contractorID = mContractorsManager->contractorIDByAddress(stepPathDebtors[1]);
        if (contractorID == ContractorsManager::kNotFoundContractorID) {
            warning() << "There is no contractor with address " << stepPathDebtors[1]->fullAddress();
            continue;
        }
        debtorsStepFlow = mTrustLinesManager->balance(contractorID);
        //  If it is Creditors branch - skip it
        if (debtorsStepFlow >= TrustLine::kZeroBalance()) {
            continue;
        }
        if (stepPathDebtors.size() != 3) {
            warning() << "Received message contains " << stepPathDebtors.size() << " debtor nodes";
            continue;
        }

        //  It has to be exactly nodes count in path
        for (const auto &nodeAddress: message->boundaryNodes()) {
            //  Prevent loop on cycles path
            if (nodeAddress == stepPathDebtors.front()) {
                continue;
            }

            auto nodeAddressAndPathRange = creditors.equal_range(nodeAddress->fullAddress());
            for (auto nodeAddressAndPathIt = nodeAddressAndPathRange.first;
                 nodeAddressAndPathIt != nodeAddressAndPathRange.second; ++nodeAddressAndPathIt) {
                if ((nodeAddressAndPathIt->second.back() == stepPathDebtors[1]) or
                    (nodeAddressAndPathIt->second.back() == stepPathDebtors[2]))
                    continue;
                vector <BaseAddress::Shared> stepCyclePath = {
                        nodeAddressAndPathIt->second.back(),
                        nodeAddress,
                        stepPathDebtors[2],
                        stepPathDebtors[1]};

                const auto cyclePath = make_shared<Path>(
                    stepCyclePath);
                mCyclesManager->addCycle(
                    cyclePath);

#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
                resultCycles.push_back(cyclePath);
#endif
            }
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
