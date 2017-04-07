#ifndef GEO_NETWORK_CLIENT_RESULTROUTINGTABLE2LEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTROUTINGTABLE2LEVELMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"
//TODO: remove after testing
#include "../../../common/time/TimeUtils.h"

#include <vector>
#include <unordered_map>

class ResultRoutingTable2LevelMessage : public TransactionMessage {

public:
    typedef shared_ptr<ResultRoutingTable2LevelMessage> Shared;

public:

    ResultRoutingTable2LevelMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID,
        unordered_map<NodeUUID, vector<NodeUUID>> &rt2);

    ResultRoutingTable2LevelMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    unordered_map<NodeUUID, vector<NodeUUID>>& rt2();

    pair<BytesShared, size_t> serializeToBytes();

private:

    typedef uint32_t RecordNumber;
    typedef RecordNumber RecordCount;

private:

    size_t rt2ByteSize();

    void deserializeFromBytes(
        BytesShared buffer);

private:

    unordered_map<NodeUUID, vector<NodeUUID>> mRT2;

};


#endif //GEO_NETWORK_CLIENT_RESULTROUTINGTABLE2LEVELMESSAGE_H
