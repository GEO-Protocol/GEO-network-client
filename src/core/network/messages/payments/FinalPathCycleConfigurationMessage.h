#ifndef GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H

#include "base/RequestCycleMessage.h"
#include "../../../payments/reservations/AmountReservation.h"
#include <map>

class FinalPathCycleConfigurationMessage :
    public RequestCycleMessage {

public:
    typedef shared_ptr<FinalPathCycleConfigurationMessage> Shared;

public:
    FinalPathCycleConfigurationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount,
        const map<NodeUUID, PaymentNodeID> &paymentNodesIds);

    FinalPathCycleConfigurationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount,
        const map<NodeUUID, PaymentNodeID> &paymentNodesIds,
        const vector<pair<PathID, AmountReservation::ConstShared>> &reservations);

    FinalPathCycleConfigurationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const map<NodeUUID, PaymentNodeID>& paymentNodesIds() const;

    const vector<pair<PathID, AmountReservation::ConstShared>> &reservations() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

private:
    map<NodeUUID, PaymentNodeID> mPaymentNodesIds;
    vector<pair<PathID, AmountReservation::ConstShared>> mReservations;
};


#endif //GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H
