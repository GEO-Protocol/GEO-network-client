/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
