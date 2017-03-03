#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEOUTGOINGMESSAGE_H

#include "../../SenderMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include <memory>
#include <utility>
#include <stdint.h>

class RoutingTableUpdateOutgoingMessage : public SenderMessage {
public:
    typedef shared_ptr<RoutingTableUpdateOutgoingMessage> Shared;

public:
    RoutingTableUpdateOutgoingMessage(
        const NodeUUID& senderUUID,
        const NodeUUID& initiatorUUID,
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction);

    const NodeUUID& initiatorUUID() const;

    const NodeUUID& contractorUUID() const;

    const TrustLineDirection direction() const;

private:
    virtual const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    NodeUUID mInitiatorUUID;
    NodeUUID mContractorUUID;
    TrustLineDirection mDirection;
};

#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEOUTGOINGMESSAGE_H
