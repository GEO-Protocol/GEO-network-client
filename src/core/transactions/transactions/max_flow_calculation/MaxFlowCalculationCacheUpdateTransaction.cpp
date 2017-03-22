#include "MaxFlowCalculationCacheUpdateTransaction.h"

MaxFlowCalculationCacheUpdateTransaction::MaxFlowCalculationCacheUpdateTransaction(
    NodeUUID &nodeUUID,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationCacheUpdateTransactionType,
        nodeUUID,
        logger),

    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager) {}

TransactionResult::SharedConst MaxFlowCalculationCacheUpdateTransaction::run() {

#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
    info() << "update cache";
#endif
    mMaxFlowCalculationCacheManager->updateCaches();
    mMaxFlowCalculationTrustLineManager->deleteLegacyTrustLines();
    return make_shared<TransactionResult>(TransactionState::exit());

}

const string MaxFlowCalculationCacheUpdateTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationCacheUpdateTA]";

    return s.str();
}