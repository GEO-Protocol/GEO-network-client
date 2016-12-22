#include "BaseTransaction.h"

BaseTransaction::BaseTransaction(
        BaseTransaction::TransactionType type) :
        mType(type) {}

const BaseTransaction::TransactionType BaseTransaction::transactionType() const {
    return mType;
}
