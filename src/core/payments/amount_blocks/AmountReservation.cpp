#include "AmountReservation.h"


AmountReservation::AmountReservation(
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const ReservationDirection direction):

    mTransactionUUID(transactionUUID),
    mBlockedAmount(amount),
    mDirection(direction)
{}

const TrustLineAmount &AmountReservation::amount() const
{
    return mBlockedAmount;
}

const TransactionUUID &AmountReservation::transactionUUID() const
{
    return mTransactionUUID;
}

const AmountReservation::ReservationDirection AmountReservation::direction() const
{
    return mDirection;
}

void AmountReservation::operator=(const AmountReservation &rhs)
{
    mBlockedAmount = rhs.mBlockedAmount;
    mTransactionUUID = rhs.mTransactionUUID;
    mDirection = rhs.mDirection;
}

bool AmountReservation::operator==(const AmountReservation &rhs) const
{
    return mBlockedAmount == rhs.mBlockedAmount &&
           mTransactionUUID == rhs.mTransactionUUID &&
           mDirection == rhs.mDirection;
}

bool AmountReservation::operator!=(const AmountReservation &rhs) const
{
    return !(rhs == *this);
}
