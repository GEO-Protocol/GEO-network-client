#include "CyclesFiveNodesReceiverTransaction.h"

CyclesFiveNodesReceiverTransaction::CyclesFiveNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFiveNodesInBetweenMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::Cycles_FiveNodesReceiverTransaction,
        nodeUUID,
        message->equivalent(),
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mInBetweenNodeTopologyMessage(message)
{}

TransactionResult::SharedConst CyclesFiveNodesReceiverTransaction::run()
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
    bool creditorsBranch = true;
    if (contractorBalance > TrustLine::kZeroBalance()) {
        creditorsBranch = false;
    }
    auto firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(creditorsBranch);
    if (firstLevelNodes.empty()){
        info() << "CyclesFiveNodesReceiverTransaction: No suitable firstLevelNodes " << endl;
        return resultDone();
    }

    auto currentDepth = (SerializedPathLengthSize)path.size();
    if (creditorsBranch and currentDepth == 1) {
        mInBetweenNodeTopologyMessage->addNodeToPath(
            mContractorsManager->ownAddresses().at(0));
        for(const auto &neighborID: firstLevelNodes)
            sendMessage(
                neighborID,
                mInBetweenNodeTopologyMessage);
        return resultDone();
    }
    if ((creditorsBranch and currentDepth==2) or (not creditorsBranch and currentDepth==1)) {
        path.push_back(
            mContractorsManager->ownAddresses().at(0));
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
        return resultDone();
    }
    else {
        return resultDone();
    }

}

const string CyclesFiveNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFiveNodesReceiverTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
