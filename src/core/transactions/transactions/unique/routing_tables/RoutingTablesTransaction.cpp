#include "RoutingTablesTransaction.h"

RoutingTablesTransaction::RoutingTablesTransaction(
    BaseTransaction::TransactionType type,
    NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const TrustLineUUID &trustLineUUID,
    TransactionsScheduler *scheduler) :

    UniqueTransaction(
        type,
        nodeUUID,
        scheduler
    ),
    mContractorUUID(contractorUUID),
    mTrustLineUUID(trustLineUUID) {}

/**
 * @Deprecated
 */
RoutingTablesTransaction::RoutingTablesTransaction(
    BaseTransaction::TransactionType type,
    NodeUUID &nodeUUID,
    TransactionsScheduler *scheduler) :

    UniqueTransaction(
        type,
        nodeUUID,
        scheduler
    ) {}

RoutingTablesTransaction::RoutingTablesTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler) :

    UniqueTransaction(scheduler) {

    deserializeFromBytes(buffer);
}

const NodeUUID &RoutingTablesTransaction::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineUUID &RoutingTablesTransaction::trustLineUUID() const {

    return mTrustLineUUID;
}

void RoutingTablesTransaction::increaseRequestsCounter() {

    mRequestCounter += 1;
}

void RoutingTablesTransaction::resetRequestsCounter() {

    mRequestCounter = 0;
}

void RoutingTablesTransaction::progressConnectionTimeout() {

    mConnectionTimeout = mConnectionTimeout * kConnectionProgression;
}

void RoutingTablesTransaction::restoreStandardConnectionTimeout() {

    mConnectionTimeout = kStandardConnectionTimeout;
}

TransactionResult::SharedConst RoutingTablesTransaction::finishTransaction() {

    return make_shared<const TransactionResult>(
        TransactionState::exit()
    );
}

pair<BytesShared, size_t> RoutingTablesTransaction::serializeToBytes() const {}

void RoutingTablesTransaction::deserializeFromBytes(
    BytesShared buffer) {}

const size_t RoutingTablesTransaction::kOffsetToDataBytes() {}