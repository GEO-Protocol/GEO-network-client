#ifndef GEO_NETWORK_CLIENT_RESULTROUTINGTABLE1LEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTROUTINGTABLE1LEVELMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

#include "../../../common/multiprecision/MultiprecisionUtils.h"

//TODO: remove after testing
#include "../../../common/time/TimeUtils.h"
#include <vector>


class ResultRoutingTable1LevelMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<ResultRoutingTable1LevelMessage> Shared;

public:
    ResultRoutingTable1LevelMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID,
        vector<NodeUUID> &rt1);

    ResultRoutingTable1LevelMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    vector<NodeUUID>& rt1();

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

protected:
    typedef uint32_t RecordNumber;
    typedef RecordNumber RecordCount;

private:
    vector<NodeUUID> mRT1;
};


#endif //GEO_NETWORK_CLIENT_RESULTROUTINGTABLE1LEVELMESSAGE_H
