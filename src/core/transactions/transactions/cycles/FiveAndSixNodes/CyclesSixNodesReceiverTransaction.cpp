#include "CyclesSixNodesReceiverTransaction.h"

CyclesSixNodesReceiverTransaction::CyclesSixNodesReceiverTransaction(
    CyclesSixNodesInBetweenMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::Cycles_SixNodesReceiverTransaction,
        message->equivalent(),
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mInBetweenNodeTopologyMessage(message)
{}

TransactionResult::SharedConst CyclesSixNodesReceiverTransaction::run()
{
    info() << "Neighbor " << mInBetweenNodeTopologyMessage->idOnReceiverSide << " sent request";
    auto contractorID = mInBetweenNodeTopologyMessage->idOnReceiverSide;
    if (!mContractorsManager->contractorPresent(contractorID)) {
        warning() << "There is no contractor " << contractorID;
        return resultDone();
    }
    vector<BaseAddress::Shared> path = mInBetweenNodeTopologyMessage->path();
    if (!mTrustLinesManager->trustLineIsActive(contractorID)) {
        warning() << "TL with previous node " << contractorID << " is not active";
        return resultDone();
    }

    auto contractorBalance = mTrustLinesManager->balance(contractorID);

    //  If balance to previous node equal zero finish transaction
    if (contractorBalance == TrustLine::kZeroBalance()) {
        return resultDone();
    }
    auto firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(
        contractorBalance < TrustLine::kZeroBalance());
    if (firstLevelNodes.empty()) {
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
        debug() << "No suitable firstLevelNodes";
#endif
        return resultDone();
    }
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    stringstream ss;
    ss << "suitable neighbors: ";
    for(const auto &neighborID: firstLevelNodes) {
        ss << neighborID << " ";
    }
    debug() << ss.str();
#endif

    //  Update message path and send to next level nodes
    const auto kCurrentDepth = (SerializedPathLengthSize)path.size();
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    info() << "current depth: " << (uint16_t)kCurrentDepth;
#endif
    path.push_back(
        mContractorsManager->selfContractor()->mainAddress());
    if (kCurrentDepth == 1) {
        for(const auto &neighborID: firstLevelNodes) {
            sendMessage<CyclesSixNodesInBetweenMessage>(
                neighborID,
                mEquivalent,
                mContractorsManager->idOnContractorSide(neighborID),
                path);
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
            debug() << "send request message to neighbor " << neighborID;
#endif
        }
    }
    else if (kCurrentDepth == 2) {
        vector<BaseAddress::Shared> boundaryNodes;
        for (const auto &neighborID: firstLevelNodes) {
            boundaryNodes.push_back(
                mContractorsManager->contractorMainAddress(neighborID));
        }
        sendMessage<CyclesSixNodesBoundaryMessage>(
            path.front(),
            mEquivalent,
            path,
            boundaryNodes);
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
        debug() << "send response message to " << path.front()->fullAddress();
#endif
    }
    else {
        warning() << "Wrong path size " << (uint16_t)kCurrentDepth;
    }
    return resultDone();
}

const string CyclesSixNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesSixNodesReceiverTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}