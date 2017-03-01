#include "MaxFlowCalculationCacheUpdateTransaction.h"

MaxFlowCalculationCacheUpdateTransaction::MaxFlowCalculationCacheUpdateTransaction(
    NodeUUID &nodeUUID,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationCacheUpdateTransactionType,
        nodeUUID),

    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mLog(logger) {}

TransactionResult::SharedConst MaxFlowCalculationCacheUpdateTransaction::run() {

    mMaxFlowCalculationCacheManager->updateCaches();

}