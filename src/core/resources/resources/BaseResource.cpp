#include "BaseResource.h"

BaseResource::BaseResource(
    const BaseResource::ResourceType type,
    const TransactionUUID &transactionUUID) :

    mType(type),
    mTransactionUUID(transactionUUID) {}

const BaseResource::ResourceType BaseResource::type() const {

    return mType;
}

const TransactionUUID &BaseResource::transactionUUID() const {

    return mTransactionUUID;
}




