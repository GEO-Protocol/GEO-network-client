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
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID& transactionUUID,
        const TrustLineAmount& amount,
        BaseAddress::Shared nextNodeInThePathAddress);

    CoordinatorCycleReservationRequestMessage(
        BytesShared buffer);

    BaseAddress::Shared nextNodeInPathAddress() const;

    const Message::MessageType typeID() const override;

    virtual pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    BaseAddress::Shared mNextPathNodeAddress;
};


#endif //GEO_NETWORK_CLIENT_COORDINATORCYCLERESERVATIONREQUESTMESSAGE_H
