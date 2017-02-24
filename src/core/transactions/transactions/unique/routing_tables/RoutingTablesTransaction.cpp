#include "RoutingTablesTransaction.h"

RoutingTablesTransaction::RoutingTablesTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID) :

    BaseTransaction(
        type,
        nodeUUID
    ),
    mContractorUUID(contractorUUID) {}


RoutingTablesTransaction::RoutingTablesTransaction(
    const TransactionType type,
    BytesShared buffer) :

    BaseTransaction(
        type
    ) {

    deserializeFromBytes(buffer);
}


const NodeUUID &RoutingTablesTransaction::contractorUUID() const {

    return mContractorUUID;
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