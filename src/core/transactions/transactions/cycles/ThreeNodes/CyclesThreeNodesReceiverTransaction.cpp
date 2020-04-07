#include "CyclesThreeNodesReceiverTransaction.h"

CyclesThreeNodesReceiverTransaction::CyclesThreeNodesReceiverTransaction(
    CyclesThreeNodesBalancesRequestMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::Cycles_ThreeNodesReceiverTransaction,
        message->transactionUUID(),
        message->equivalent(),
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mRequestMessage(message)
{}

TransactionResult::SharedConst CyclesThreeNodesReceiverTransaction::run()
{
    info() << "Neighbor " << mRequestMessage->idOnReceiverSide << " send request with "
           << mRequestMessage->neighbors().size() << " nodes to check";
    if (!mTrustLinesManager->trustLineIsActive(mRequestMessage->idOnReceiverSide)) {
        warning() << "TL with requested node " << mRequestMessage->idOnReceiverSide << " is not active";
        return resultDone();
    }
    const auto kNeighbors = mRequestMessage->neighbors();
    const auto kContractorBalance = mTrustLinesManager->balance(mRequestMessage->idOnReceiverSide);

    if (kContractorBalance == TrustLine::kZeroBalance()) {
        return resultDone();
    }

    vector<BaseAddress::Shared> commonNeighbors;

    bool searchDebtors = true;
    if (kContractorBalance > TrustLine::kZeroBalance())
        searchDebtors = false;

    TrustLineBalance stepNodeBalance;
    for (const auto &kNodeAddress: kNeighbors) {
        auto neighborID = mContractorsManager->contractorIDByAddress(kNodeAddress);
        if (neighborID == ContractorsManager::kNotFoundContractorID) {
            warning() << "There is no neighbor with address " << kNodeAddress->fullAddress();
            continue;
        }
        if (!mTrustLinesManager->trustLineIsActive(neighborID)) {
            continue;
        }
        stepNodeBalance = mTrustLinesManager->balance(neighborID);
        if ((searchDebtors and stepNodeBalance > TrustLine::kZeroBalance())
            or (not searchDebtors and (stepNodeBalance < TrustLine::kZeroBalance())))
            commonNeighbors.push_back(
                kNodeAddress);
    }
    if (!commonNeighbors.empty()) {
        sendMessage<CyclesThreeNodesBalancesResponseMessage>(
            mRequestMessage->idOnReceiverSide,
            mEquivalent,
            mContractorsManager->idOnContractorSide(
                mRequestMessage->idOnReceiverSide),
            currentTransactionUUID(),
            commonNeighbors);
    } else {
#ifdef DEBUG_LOG_CYCLES_BUILDING_POCESSING
        debug() << "There are no suitable nodes";
#endif
    }
    return resultDone();
}

const string CyclesThreeNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesThreeNodesReceiverTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}

