#include "CyclesFourNodesReceiverTransaction.h"

CyclesFourNodesReceiverTransaction::CyclesFourNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFourNodesNegativeBalanceRequestMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::Cycles_FourNodesReceiverTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mRequestMessage(message),
    mNegativeCycleBalance(true)
{}

CyclesFourNodesReceiverTransaction::CyclesFourNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFourNodesPositiveBalanceRequestMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::Cycles_FourNodesReceiverTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mRequestMessage(message),
    mNegativeCycleBalance(false)
{}

TransactionResult::SharedConst CyclesFourNodesReceiverTransaction::run()
{
    info() << "Neighbor " << mRequestMessage->contractorAddress()->fullAddress() << " send request with "
           << mRequestMessage->checkedNodes().size() << " to check";
    mNeighborID = mContractorsManager->contractorIDByAddress(
        mRequestMessage->contractorAddress());
    if (mNeighborID == ContractorsManager::kNotFoundContractorID) {
        warning() << "Neighbor " << mRequestMessage->contractorAddress()->fullAddress() << " is not a neighbor";
        return resultDone();
    }
    info() << "Neighbor ID " << mNeighborID;
    if (!mTrustLinesManager->trustLineIsPresent(mNeighborID)) {
        warning() << "There is no TL with neighbor " << mRequestMessage->contractorAddress()->fullAddress();
        return resultDone();
    }

    if (mNegativeCycleBalance) {
        buildSuitableDebtorsForCycleNegativeBalance();
    } else {
        buildSuitableDebtorsForCyclePositiveBalance();
    }

    if (!mSuitableNodes.empty()) {
        sendMessage<CyclesFourNodesBalancesResponseMessage>(
            mRequestMessage->senderAddresses.at(0),
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            currentTransactionUUID(),
            mSuitableNodes);
    } else {
        info() << "There are no suitable nodes";
    }

    return resultDone();
}

void CyclesFourNodesReceiverTransaction::buildSuitableDebtorsForCycleNegativeBalance()
{
    auto creditorBalance = mTrustLinesManager->balance(mNeighborID);
    if (creditorBalance <= TrustLine::kZeroBalance()) {
        info() << "Not positive balance with contractor node";
        return;
    }

    for (const auto &checkedNode : mRequestMessage->checkedNodes()) {
        auto checkedContractorID = mContractorsManager->contractorIDByAddress(
            checkedNode);
        if (checkedContractorID == ContractorsManager::kNotFoundContractorID) {
            warning() << "Checked node " << checkedNode->fullAddress() << " is not a neighbor";
            continue;
        }
        if (!mTrustLinesManager->trustLineIsPresent(checkedContractorID)) {
            warning() << "There is no TL with checked node " << checkedNode->fullAddress();
            continue;
        }
        if (!mTrustLinesManager->trustLineIsActive(checkedContractorID)) {
            warning() << "TL with checked node " << checkedNode->fullAddress() << " is not active";
            continue;
        }
        if (mTrustLinesManager->balance(checkedContractorID) < TrustLine::kZeroBalance()) {
            mSuitableNodes.push_back(checkedNode);
        }
    }
}

void CyclesFourNodesReceiverTransaction::buildSuitableDebtorsForCyclePositiveBalance()
{
    auto creditorBalance = mTrustLinesManager->balance(mNeighborID);
    if (creditorBalance >= TrustLine::kZeroBalance()) {
        info() << "Not negative balance with contractor node";
        return;
    }

    for (const auto &checkedNode : mRequestMessage->checkedNodes()) {
        auto checkedContractorID = mContractorsManager->contractorIDByAddress(
            checkedNode);
        if (checkedContractorID == ContractorsManager::kNotFoundContractorID) {
            warning() << "Checked node " << checkedNode->fullAddress() << " is not a neighbor";
            continue;
        }
        if (!mTrustLinesManager->trustLineIsPresent(checkedContractorID)) {
            warning() << "There is no TL with checked node " << checkedNode->fullAddress();
            continue;
        }
        if (!mTrustLinesManager->trustLineIsActive(checkedContractorID)) {
            warning() << "TL with checked node " << checkedNode->fullAddress() << " is not active";
            continue;
        }
        if (mTrustLinesManager->balance(checkedContractorID) > TrustLine::kZeroBalance()) {
            mSuitableNodes.push_back(checkedNode);
        }
    }
}

const string CyclesFourNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFourNodesReceiverTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}