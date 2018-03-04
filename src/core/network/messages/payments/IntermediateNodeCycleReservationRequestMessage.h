#ifndef GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H

#include "base/RequestCycleMessage.h"

class IntermediateNodeCycleReservationRequestMessage :
    public RequestCycleMessage {

public:
    typedef shared_ptr<IntermediateNodeCycleReservationRequestMessage> Shared;
    typedef shared_ptr<const IntermediateNodeCycleReservationRequestMessage> ConstShared;

public:
    IntermediateNodeCycleReservationRequestMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID& senderUUID,
        const TransactionUUID& transactionUUID,
        const TrustLineAmount& amount,
        const NodeUUID& coordinatorUUID,
        SerializedPathLengthSize cucleLength);

    IntermediateNodeCycleReservationRequestMessage(
        BytesShared buffer);

    const NodeUUID& coordinatorUUID() const;

    SerializedPathLengthSize cycleLength() const;

protected:
    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

protected:
    SerializedPathLengthSize mCycleLength;
    NodeUUID mCoordinatorUUID;
};


#endif //GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H
