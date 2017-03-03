#include "MaxFlowCalculationCacheUpdateTransaction.h"

MaxFlowCalculationCacheUpdateTransaction::MaxFlowCalculationCacheUpdateTransaction(
    NodeUUID &nodeUUID,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationCacheUpdateTransactionType,
        nodeUUID),

    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mLog(logger) {}

TransactionResult::SharedConst MaxFlowCalculationCacheUpdateTransaction::run() {

    mLog->logInfo("MaxFlowCalculationCacheUpdateTransaction", "update cache");
    mMaxFlowCalculationCacheManager->updateCaches();
    mMaxFlowCalculationTrustLineManager->deleteLegacyTrustLines();
    return make_shared<TransactionResult>(TransactionState::exit());

}