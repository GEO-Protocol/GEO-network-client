#include "TrustLineTransaction.h"

TrustLineTransaction::TrustLineTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID,
    Logger &logger) :

    BaseTransaction(
        type,
        nodeUUID,
        logger) {

    setExpectationResponsesCounter(kResponsesCount);
}

TrustLineTransaction::TrustLineTransaction(
    const BaseTransaction::TransactionType type,
    Logger &logger) :

    BaseTransaction(
        type,
        logger) {

    setExpectationResponsesCounter(kResponsesCount);
}