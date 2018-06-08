#ifndef GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H

#include "base/RequestMessageWithReservations.h"
#include "../../../payments/reservations/AmountReservation.h"
#include <map>

class FinalAmountsConfigurationMessage : public RequestMessageWithReservations {

public:
    typedef shared_ptr<FinalAmountsConfigurationMessage> Shared;

public:
    FinalAmountsConfigurationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
        const map<NodeUUID, PaymentNodeID> &paymentNodesIds);

    // if coordinator has reservation with current node it also send them
    FinalAmountsConfigurationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
        const map<NodeUUID, PaymentNodeID> &paymentNodesIds,
        const vector<pair<PathID, AmountReservation::ConstShared>> &reservations);

    FinalAmountsConfigurationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const map<NodeUUID, PaymentNodeID> &paymentNodesIds() const;

    const vector<pair<PathID, AmountReservation::ConstShared>> &reservations() const;

protected:
    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

private:
    map<NodeUUID, PaymentNodeID> mPaymentNodesIds;
    vector<pair<PathID, AmountReservation::ConstShared>> mReservations;
};


#endif //GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H
