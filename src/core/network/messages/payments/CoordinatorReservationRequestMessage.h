#ifndef COORDINATORRESERVATIONREQUESTMESSAGE_H
#define COORDINATORRESERVATIONREQUESTMESSAGE_H

#include "base/RequestMessageWithReservations.h"

class CoordinatorReservationRequestMessage:
    public RequestMessageWithReservations {

public:
    typedef shared_ptr<CoordinatorReservationRequestMessage> Shared;
    typedef shared_ptr<const CoordinatorReservationRequestMessage> ConstShared;

public:
    CoordinatorReservationRequestMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID& senderUUID,
        const TransactionUUID& transactionUUID,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
        const NodeUUID& nextNodeInThePath);

    CoordinatorReservationRequestMessage(
        BytesShared buffer);

    const NodeUUID& nextNodeInPath() const;

    const Message::MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

protected:
     NodeUUID mNextPathNode;
};

#endif // COORDINATORRESERVATIONREQUESTMESSAGE_H
