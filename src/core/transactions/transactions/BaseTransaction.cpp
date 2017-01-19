#include "BaseTransaction.h"

BaseTransaction::BaseTransaction(
        BaseTransaction::TransactionType type,
        NodeUUID &nodeUUID) :

        mType(type),
        mNodeUUID(nodeUUID){}

const BaseTransaction::TransactionType BaseTransaction::transactionType() const {
    return mType;
}

const NodeUUID &BaseTransaction::nodeUUID() const {
    return mNodeUUID;
}

const TransactionUUID &BaseTransaction::transactionUUID() const {
    return mTransactionUUID;
}

void BaseTransaction::setContext(
    Message::Shared message) {

    mContext = message;
}

pair<ConstBytesShared, size_t> BaseTransaction::serializeContext() {
    return mContext->serialize();
}

void BaseTransaction::increaseStepsCounter() {

    mStep += 1;
}

void BaseTransaction::increaseRequestsCounter() {

    mRequestCounter += 1;
}
