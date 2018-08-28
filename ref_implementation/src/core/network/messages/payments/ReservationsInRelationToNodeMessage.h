/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_RESERVATIONSINRELATIONTONODEMESSAGE_H
#define GEO_NETWORK_CLIENT_RESERVATIONSINRELATIONTONODEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../payments/reservations/AmountReservation.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

class ReservationsInRelationToNodeMessage : public TransactionMessage {

public:
    typedef shared_ptr<ReservationsInRelationToNodeMessage> Shared;
    typedef shared_ptr<const ReservationsInRelationToNodeMessage> ConstShared;

public:
    ReservationsInRelationToNodeMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathID, AmountReservation::ConstShared>> &reservations);

    ReservationsInRelationToNodeMessage(
        BytesShared buffer);

    const vector<pair<PathID, AmountReservation::ConstShared>> &reservations() const;

protected:
    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

private:
    vector<pair<PathID, AmountReservation::ConstShared>> mReservations;
};


#endif //GEO_NETWORK_CLIENT_RESERVATIONSINRELATIONTONODEMESSAGE_H
