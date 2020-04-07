#include "CyclesFiveNodesReceiverTransaction.h"

CyclesFiveNodesReceiverTransaction::CyclesFiveNodesReceiverTransaction(
    CyclesFiveNodesInBetweenMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::Cycles_FiveNodesReceiverTransaction,
        message->equivalent(),
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mInBetweenNodeTopologyMessage(message)
{}

TransactionResult::SharedConst CyclesFiveNodesReceiverTransaction::run()
{
    info() << "Neighbor " << mInBetweenNodeTopologyMessage->idOnReceiverSide << " sent request";
    auto contractorID = mInBetweenNodeTopologyMessage->idOnReceiverSide;
    if (!mContractorsManager->contractorPresent(contractorID)) {
        warning() << "There is no contractor " << contractorID;
        return resultDone();
    }
    auto path = mInBetweenNodeTopologyMessage->path();
    if (!mTrustLinesManager->trustLineIsActive(contractorID)) {
        warning() << "TL with previous node " << contractorID << " is not active";
        return resultDone();
    }

    auto contractorBalance = mTrustLinesManager->balance(contractorID);
    //  If balance to previous node equal zero finish transaction
    if (contractorBalance == TrustLine::kZeroBalance()) {
        return resultDone();
    }
    bool isCreditorsBranch = true;
    if (contractorBalance > TrustLine::kZeroBalance()) {
        isCreditorsBranch = false;
    }
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    debug() << "Creditor branch " << isCreditorsBranch;
#endif
    auto firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(isCreditorsBranch);
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

    auto currentDepth = (SerializedPathLengthSize)path.size();
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    debug() << "currentDepth " << (uint16_t)currentDepth;
#endif
    path.push_back(
        mContractorsManager->selfContractor()->mainAddress());
    if (not isCreditorsBranch and currentDepth == 1) {
        for(const auto &neighborID: firstLevelNodes) {
            sendMessage<CyclesFiveNodesInBetweenMessage>(
                neighborID,
                mEquivalent,
                mContractorsManager->idOnContractorSide(neighborID),
                path);
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
            debug() << "send request message to neighbor " << neighborID;
#endif
        }
        return resultDone();
    }
    if ((not isCreditorsBranch and currentDepth==2) or (isCreditorsBranch and currentDepth==1)) {
        vector<BaseAddress::Shared> boundaryNodes;
        for (const auto &neighborID: firstLevelNodes) {
            boundaryNodes.push_back(
                mContractorsManager->contractorMainAddress(neighborID));
        }
        sendMessage<CyclesFiveNodesBoundaryMessage>(
            path.front(),
            mEquivalent,
            path,
            boundaryNodes);
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
        debug() << "send response message to " << path.front()->fullAddress();
#endif
        return resultDone();
    }
    else {
        warning() << "wrong depth " << (uint16_t)currentDepth << " creditor branch " << isCreditorsBranch;
        return resultDone();
    }

}

const string CyclesFiveNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFiveNodesReceiverTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
