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
    auto contractorID = mInBetweenNodeTopologyMessage->idOnReceiverSide;
    if (!mContractorsManager->contractorPresent(contractorID)) {
        warning() << "There is no contractor " << contractorID;
        return resultDone();
    }
    vector<BaseAddress::Shared> path = mInBetweenNodeTopologyMessage->Path();
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
    //  Update message path and send to next level nodes
    const auto kCurrentDepth = (SerializedPathLengthSize)path.size();
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
    info() << "current depth: " << to_string(kCurrentDepth);
#endif
    if (kCurrentDepth == 1) {
        mInBetweenNodeTopologyMessage->addNodeToPath(
            mContractorsManager->ownAddresses().at(0));
        for(const auto &neighborID: firstLevelNodes) {
            sendMessage(
                neighborID,
                mInBetweenNodeTopologyMessage);
        }
    }
    else if (kCurrentDepth == 2) {
        path.push_back(
            mContractorsManager->ownAddresses().at(0));
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
    }
    else {
        warning() << "Wrong path size " << to_string(kCurrentDepth);
    }
    return resultDone();
}

const string CyclesSixNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesSixNodesReceiverTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}