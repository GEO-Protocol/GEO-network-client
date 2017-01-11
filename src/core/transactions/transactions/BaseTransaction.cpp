#include "BaseTransaction.h"

BaseTransaction::BaseTransaction(
        BaseTransaction::TransactionType type) :
        mType(type) {}

const BaseTransaction::TransactionType BaseTransaction::type() const {
    return mType;
}

const TransactionUUID BaseTransaction::uuid() const {
    return mTransactionUUID;
}
