#ifndef GEO_NETWORK_CLIENT_COORDINATORCYCLERESERVATIONREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_COORDINATORCYCLERESERVATIONREQUESTMESSAGE_H

#include "base/RequestCycleMessage.h"

class CoordinatorCycleReservationRequestMessage :
    public RequestCycleMessage{

public:
    typedef shared_ptr<CoordinatorCycleReservationRequestMessage> Shared;
    typedef shared_ptr<const CoordinatorCycleReservationRequestMessage> ConstShared;

public:
    CoordinatorCycleReservationRequestMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID& senderUUID,
        const TransactionUUID& transactionUUID,
        const TrustLineAmount& amount,
        const NodeUUID& nextNodeInThePath);

    CoordinatorCycleReservationRequestMessage(
        BytesShared buffer);

    const NodeUUID& nextNodeInPath() const;

    const Message::MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

protected:
    NodeUUID mNextPathNode;
};


#endif //GEO_NETWORK_CLIENT_COORDINATORCYCLERESERVATIONREQUESTMESSAGE_H
