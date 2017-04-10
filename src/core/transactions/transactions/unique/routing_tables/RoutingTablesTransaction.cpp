#include "RoutingTablesTransaction.h"

RoutingTablesTransaction::RoutingTablesTransaction(
    const TransactionType type,
    BytesShared buffer,
    Logger *logger) :

    BaseTransaction(
        type,
        logger) {

    deserializeFromBytes(buffer);
}

RoutingTablesTransaction::RoutingTablesTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID) :

    BaseTransaction(
        type,
        nodeUUID) {}

RoutingTablesTransaction::RoutingTablesTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    Logger *logger) :

    BaseTransaction(
        type,
        nodeUUID,
        logger),
    mContractorUUID(contractorUUID) {}


const NodeUUID &RoutingTablesTransaction::contractorUUID() const {

    return mContractorUUID;
}

const vector<NodeUUID> &RoutingTablesTransaction::contractorsUUIDs() const {

    return mContractorsUUIDs;
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

pair<BytesShared, size_t> RoutingTablesTransaction::serializeToBytes() const {}

void RoutingTablesTransaction::deserializeFromBytes(
    BytesShared buffer) {}

const size_t RoutingTablesTransaction::kOffsetToDataBytes() {}