#include "CyclesThreeNodesInitTransaction.h"

CyclesThreeNodesInitTransaction::CyclesThreeNodesInitTransaction(
    ContractorID contractorID,
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    RoutingTableManager *routingTable,
    CyclesManager *cyclesManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::Cycles_ThreeNodesInitTransaction,
        equivalent,
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mCyclesManager(cyclesManager),
    mContractorID(contractorID),
    mRougingTable(routingTable)
{}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::run()
{
    switch (mStep){
        case Stages::CollectDataAndSendMessage:
            return runCollectDataAndSendMessageStage();

        case Stages::ParseMessageAndCreateCycles:
            return runParseMessageAndCreateCyclesStage();

        default:
            throw RuntimeError(
                "CycleThreeNodesInitTransaction::run(): "
                "invalid transaction step.");
    }
}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::runCollectDataAndSendMessageStage()
{
    debug() << "runCollectDataAndSendMessageStage to " << mContractorID;
    if (!mTrustLinesManager->trustLineIsActive(mContractorID)) {
        warning() << "TL with contractor is not active";
        return resultDone();
    }
    auto commonNeighbors = getNeighborsWithContractor();
    if(commonNeighbors.empty()){
        info() << "No common neighbors with: " << mContractorID;
        return resultDone();
    }
    sendMessage<CyclesThreeNodesBalancesRequestMessage>(
        mContractorID,
        mEquivalent,
        mContractorsManager->idOnContractorSide(mContractorID),
        currentTransactionUUID(),
        commonNeighbors);
    mStep = Stages::ParseMessageAndCreateCycles;
    return resultWaitForMessageTypes(
        {Message::Cycles_ThreeNodesBalancesResponse},
        mkWaitingForResponseTime);
}

TransactionResult::SharedConst CyclesThreeNodesInitTransaction::runParseMessageAndCreateCyclesStage()
{
    debug() << "runParseMessageAndCreateCyclesStage";
    if (mContext.empty()){
        info() << "No responses messages are present. Can't create cycles paths;";
        return resultDone();
    }
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    vector<Path::Shared> resultCycles;
#endif
    auto message = popNextMessage<CyclesThreeNodesBalancesResponseMessage>();
    if (message->commonNodes().empty()) {
        info() << "There are no suitable nodes in response";
        return resultDone();
    }
    for(const auto &nodeAddress : message->commonNodes() ){
        vector<BaseAddress::Shared> cycle = {
            nodeAddress,
            mContractorsManager->contractorMainAddress(mContractorID)};
        if (mTrustLinesManager->balance(mContractorID) > TrustLine::kZeroBalance()) {
            reverse(
                cycle.begin(),
                cycle.end());
        }
        // Path object is common object.
        const auto cyclePath = make_shared<Path>(
            cycle);
        mCyclesManager->addCycle(
            cyclePath);
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
            resultCycles.push_back(cyclePath);
#endif
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

vector<BaseAddress::Shared> CyclesThreeNodesInitTransaction::getNeighborsWithContractor()
{
    vector<BaseAddress::Shared> ownNeighbors, commonNeighbors;
    const auto kBalanceToContractor = mTrustLinesManager->balance(mContractorID);
    if (kBalanceToContractor == TrustLine::kZeroBalance()) {
        return commonNeighbors;
    }

    for (const auto &kNodeUUIDAndTrustLine: mTrustLinesManager->trustLines()) {
        const auto kTL = kNodeUUIDAndTrustLine.second;
        if (kTL->state() != TrustLine::Active) {
            continue;
        }
        if (kBalanceToContractor < TrustLine::kZeroBalance()) {
            if (kTL->balance() > TrustLine::kZeroBalance())
                ownNeighbors.push_back(
                    mContractorsManager->contractorMainAddress(
                        kNodeUUIDAndTrustLine.first));
        }
        else if (kTL->balance() < TrustLine::kZeroBalance())
            ownNeighbors.push_back(
                mContractorsManager->contractorMainAddress(
                    kNodeUUIDAndTrustLine.first));
    }
    const auto contractorNeighbors = mRougingTable->secondLevelContractorsForNode(
        mContractorID);

    for (const auto &ownNeighbor : ownNeighbors) {
        for (const auto &contractorNeighbor : contractorNeighbors) {
            if (ownNeighbor == contractorNeighbor) {
                commonNeighbors.push_back(ownNeighbor);
            }
        }
    }
    // todo : try to use set and set_intersection

    return commonNeighbors;
}

const string CyclesThreeNodesInitTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesThreeNodesInitTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
