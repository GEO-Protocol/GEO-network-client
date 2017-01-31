#ifndef GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEOUTGOINGMESSAGE_H

#include "../../Message.h"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/TransactionUUID.h"

#include "../../../../common/exceptions/MemoryError.h"

#include <memory>

using namespace std;

class FirstLevelRoutingTableOutgoingMessage : public Message {
public:
    typedef shared_ptr<FirstLevelRoutingTableOutgoingMessage> Shared;

public:
    FirstLevelRoutingTableOutgoingMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        NodeUUID &contractor);

    ~FirstLevelRoutingTableOutgoingMessage();

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte *buffer);

    void pushBack(
        NodeUUID &neighbor,
        TrustLineDirection direction);

    const MessageTypeID typeID() const;

private:
    NodeUUID &mContractor;
    unique_ptr<map<NodeUUID, TrustLineDirection>> mRecords;
};


#endif //GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEOUTGOINGMESSAGE_H
