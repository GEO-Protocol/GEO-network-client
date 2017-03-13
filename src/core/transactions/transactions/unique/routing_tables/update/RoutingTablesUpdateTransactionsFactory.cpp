#include "RoutingTablesUpdateTransactionsFactory.h"

RoutingTablesUpdateTransactionsFactory::RoutingTablesUpdateTransactionsFactory(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction,
    TrustLinesManager *trustLinesManager) :

    BaseTransaction(
        BaseTransaction::TransactionType::RoutingTablesUpdatesFactoryTransactionType,
        nodeUUID),
    mContractorUUID(contractorUUID),
    mTrustLinesManager(trustLinesManager) {

    mDirection = direction;
}

TransactionResult::SharedConst RoutingTablesUpdateTransactionsFactory::run() {

    createRoutingTablesUpdatePoolTransactions();
    return finishTransaction();
}

void RoutingTablesUpdateTransactionsFactory::createRoutingTablesUpdatePoolTransactions() {

    for (const auto &contractorAndTrustLine : mTrustLinesManager->trustLines()) {

        if (mContractorUUID == contractorAndTrustLine.first) {
            continue;
        }

        BaseTransaction::Shared updatesPropagationTransaction = make_shared<PropagateRoutingTablesUpdatesTransaction>(
            mNodeUUID,
            mNodeUUID,
            mContractorUUID,
            mDirection,
            contractorAndTrustLine.first,
            RoutingTableUpdateOutgoingMessage::UpdatingStep::FirstLevelNodes);

        launchSubsidiaryTransaction(updatesPropagationTransaction);

    }
}
