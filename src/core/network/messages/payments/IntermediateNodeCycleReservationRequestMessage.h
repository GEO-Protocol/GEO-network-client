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
        const NodeUUID& senderUUID,
        const TransactionUUID& transactionUUID,
        const TrustLineAmount& amount,
        uint8_t cucleLength);

    IntermediateNodeCycleReservationRequestMessage(
        BytesShared buffer);

    uint8_t cycleLength() const;

protected:
    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

protected:
    uint8_t mCycleLength;
};


#endif //GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H
