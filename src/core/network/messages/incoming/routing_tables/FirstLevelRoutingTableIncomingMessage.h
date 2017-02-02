#ifndef GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEINCOMINGMESSAGE_H
#define GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEINCOMINGMESSAGE_H

#include "../../Message.h"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/TransactionUUID.h"

#include "../../../../common/exceptions/MemoryError.h"

#include <memory>

using namespace std;

class FirstLevelRoutingTableIncomingMessage : public Message {
public:
    typedef shared_ptr<FirstLevelRoutingTableIncomingMessage> Shared;

public:
    FirstLevelRoutingTableIncomingMessage(
        byte *buffer);

    ~FirstLevelRoutingTableIncomingMessage();

    pair<ConstBytesShared, size_t> serialize();

    const MessageTypeID typeID() const;

private:
    void deserialize(
        byte* buffer);

private:
    NodeUUID mContractor;
    unique_ptr<map<NodeUUID, TrustLineDirection>> mRecords;

};


#endif //GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEINCOMINGMESSAGE_H
