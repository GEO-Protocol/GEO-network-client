#ifndef GEO_NETWORK_CLIENT_AMOUNTBLOCK_H
#define GEO_NETWORK_CLIENT_AMOUNTBLOCK_H

#include "../../common/Types.h"
#include "../../transactions/transactions/base/TransactionUUID.h"


/*
 * Describes trust line amount reservation.
 * Used for amount locking on trust lines in payment operations
 * and cycles overlapping operations.
 */
class AmountReservation {
public:
    typedef shared_ptr<const AmountReservation> ConstShared;

public:
    AmountReservation(
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount);

    const TrustLineAmount& amount() const;
    const TransactionUUID& transactionUUID() const;

    void operator= (const AmountReservation &rhs);
    bool operator==(const AmountReservation &rhs) const;
    bool operator!=(const AmountReservation &rhs) const;

protected:
    TrustLineAmount mBlockedAmount;
    TransactionUUID mTransactionUUID;
};


#endif //GEO_NETWORK_CLIENT_AMOUNTBLOCK_H
