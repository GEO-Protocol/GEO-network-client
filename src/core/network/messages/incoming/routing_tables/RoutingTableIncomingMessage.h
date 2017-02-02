#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H

#include "../../Message.h"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/TransactionUUID.h"

#include "../../../../common/exceptions/MemoryError.h"

#include <memory>

using namespace std;

class RoutingTableIncomingMessage : public Message {
public:
    typedef shared_ptr<RoutingTableIncomingMessage> Shared;

protected:
    RoutingTableIncomingMessage(
        byte *buffer);

    ~RoutingTableIncomingMessage();

    virtual const MessageTypeID typeID() const = 0;

private:
    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte* buffer);

protected:
    NodeUUID mContractor;
    unique_ptr<map<NodeUUID, TrustLineDirection>> mRecords;

};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H
