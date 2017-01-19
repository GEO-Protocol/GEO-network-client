#include "AmountReservation.h"


AmountReservation::AmountReservation(
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount):

    mTransactionUUID(transactionUUID),
    mBlockedAmount(amount){}

const TrustLineAmount &AmountReservation::amount() const {
    return mBlockedAmount;
}

const TransactionUUID &AmountReservation::transactionUUID() const {
    return mTransactionUUID;
}

void AmountReservation::operator=(const AmountReservation &rhs) {
    mBlockedAmount = rhs.mBlockedAmount;
    mTransactionUUID = rhs.mTransactionUUID;
}

bool AmountReservation::operator==(const AmountReservation &rhs) const {
    return mBlockedAmount == rhs.mBlockedAmount &&
           mTransactionUUID == rhs.mTransactionUUID;
}

bool AmountReservation::operator!=(const AmountReservation &rhs) const {
    return !(rhs == *this);
}
