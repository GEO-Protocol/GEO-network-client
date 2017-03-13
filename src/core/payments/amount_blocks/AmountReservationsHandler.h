#ifndef GEO_NETWORK_CLIENT_AMOUNTBLOCKSHANDLER_H
#define GEO_NETWORK_CLIENT_AMOUNTBLOCKSHANDLER_H


#include "AmountReservation.h"
#include "../../common/NodeUUID.h"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../common/exceptions/MemoryError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/NotFoundError.h"

#include <map>


class AmountReservationsHandler {
public:
    AmountReservation::ConstShared reserve(
        const NodeUUID &trustLineContractor,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount,
        const AmountReservation::ReservationDirection direction);

    AmountReservation::ConstShared updateReservation(
        const NodeUUID &trustLineContractor,
        const AmountReservation::ConstShared reservation,
        const TrustLineAmount &newAmount);

    void free(
        const NodeUUID &trustLineContractor,
        const AmountReservation::ConstShared reservation);

    ConstSharedTrustLineAmount totalReserved(
        const NodeUUID &trustLineContractor,
        const AmountReservation::ReservationDirection direction,
        const TransactionUUID *transactionUUID = nullptr) const;

protected:
    // One trust line may hold several amount reservations,
    // so the vector<AmountReservation::ConstShared> is used (reservations container).
    //
    // Blocks container would be requested from the map very often,
    // but map's iterator returns copy of the object,
    // so the unique_ptr<vector> is used to get the container withoud copying it.
    //
    // unique_ptr is used because reservations will never be transafered outside of this class.
    map<NodeUUID, unique_ptr<vector<AmountReservation::ConstShared>>> mReservations;

protected:
    std::vector<AmountReservation::ConstShared> reservations(
        const NodeUUID &trustLineContractor,
        const TransactionUUID *transactionUUID = nullptr) const;
};


#endif //GEO_NETWORK_CLIENT_AMOUNTBLOCKSHANDLER_H
