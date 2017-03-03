#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEINCOMINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEINCOMINGMESSAGE_H

#include "../../SenderMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"

#include <memory>
#include <utility>
#include <stdint.h>

class RoutingTableUpdateIncomingMessage : public SenderMessage {

public:
    RoutingTableUpdateIncomingMessage(
        BytesShared buffer);

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


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATEINCOMINGMESSAGE_H
