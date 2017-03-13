#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEINCOMINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEINCOMINGMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"

#include "../../outgoing/routing_tables/RoutingTableUpdateOutgoingMessage.h"

#include <memory>
#include <utility>
#include <stdint.h>

class RoutingTableUpdateIncomingMessage : public TransactionMessage {
public:
    typedef shared_ptr<RoutingTableUpdateIncomingMessage> Shared;

public:
    RoutingTableUpdateIncomingMessage(
        BytesShared buffer);

    const NodeUUID& initiatorUUID() const;

    const NodeUUID& contractorUUID() const;

    const TrustLineDirection direction() const;

    const RoutingTableUpdateOutgoingMessage::UpdatingStep updatingStep() const;

private:
    virtual const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    NodeUUID mInitiatorUUID;
    NodeUUID mContractorUUID;
    TrustLineDirection mDirection;
    RoutingTableUpdateOutgoingMessage::UpdatingStep mUpdatingStep;
};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEINCOMINGMESSAGE_H
