#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H

#include "../../Message.h"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/TransactionUUID.h"

#include "../../../../common/exceptions/MemoryError.h"

#include <memory>

using namespace std;

class RoutingTableOutgoingMessage : public Message {
public:
    typedef shared_ptr<RoutingTableOutgoingMessage> Shared;

public:
    void pushBack(
        NodeUUID &neighbor,
        TrustLineDirection direction);

protected:
    RoutingTableOutgoingMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        NodeUUID &contractor);

    ~RoutingTableOutgoingMessage();

    virtual const MessageTypeID typeID() const = 0;

private:
    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte *buffer);

protected:
    NodeUUID &mContractor;
    unique_ptr<map<NodeUUID, TrustLineDirection>> mRecords;
};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H
