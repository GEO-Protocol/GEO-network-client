#ifndef COORDINATORRESERVATIONREQUESTMESSAGE_H
#define COORDINATORRESERVATIONREQUESTMESSAGE_H


#include "FinalAmountsConfigurationMessage.h"


class CoordinatorReservationRequestMessage:
    public FinalAmountsConfigurationMessage {

public:
    typedef shared_ptr<CoordinatorReservationRequestMessage> Shared;
    typedef shared_ptr<const CoordinatorReservationRequestMessage> ConstShared;

public:
    CoordinatorReservationRequestMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID& transactionUUID,
        const vector<pair<PathUUID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
        const NodeUUID& nextNodeInThePath);

    CoordinatorReservationRequestMessage(
        BytesShared buffer);

    const NodeUUID& nextNodeInPathUUID() const;

    const Message::MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

protected:
     NodeUUID mNextPathNode;
};

#endif // COORDINATORRESERVATIONREQUESTMESSAGE_H
