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
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathID, AmountReservation::ConstShared>> &reservations,
        PaymentNodeID paymentNodeID,
        uint32_t publicKeyHash);

    ReservationsInRelationToNodeMessage(
        BytesShared buffer);

    const vector<pair<PathID, AmountReservation::ConstShared>> &reservations() const;

    const uint32_t publicKeyHash() const;

    const PaymentNodeID paymentNodeID() const;

protected:
    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

private:
    vector<pair<PathID, AmountReservation::ConstShared>> mReservations;
    uint32_t mPublicKeyHash;
    PaymentNodeID mPaymentNodeID;
};


#endif //GEO_NETWORK_CLIENT_RESERVATIONSINRELATIONTONODEMESSAGE_H
