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
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID& transactionUUID,
        const TrustLineAmount& amount,
        BaseAddress::Shared coordinatorAddress,
        SerializedPathLengthSize cycleLength);

    IntermediateNodeCycleReservationRequestMessage(
        BytesShared buffer);

    BaseAddress::Shared coordinatorAddress() const;

    SerializedPathLengthSize cycleLength() const;

protected:
    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

protected:
    SerializedPathLengthSize mCycleLength;
    BaseAddress::Shared mCoordinatorAddress;
};


#endif //GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H
