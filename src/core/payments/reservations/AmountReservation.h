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
    enum ReservationDirection {
        Outgoing,
        Incoming
    };

    typedef uint8_t SerializedReservationDirectionSize;

public:
    AmountReservation(
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount,
        const ReservationDirection direction);

    const TrustLineAmount& amount() const;
    const TransactionUUID& transactionUUID() const;
    const ReservationDirection direction() const;

    void operator= (const AmountReservation &rhs);
    bool operator==(const AmountReservation &rhs) const;
    bool operator!=(const AmountReservation &rhs) const;

protected:
    TrustLineAmount mBlockedAmount;
    TransactionUUID mTransactionUUID;
    ReservationDirection mDirection;
};


#endif //GEO_NETWORK_CLIENT_AMOUNTBLOCK_H
