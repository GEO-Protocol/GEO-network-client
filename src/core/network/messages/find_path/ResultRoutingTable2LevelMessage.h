#ifndef GEO_NETWORK_CLIENT_RESULTROUTINGTABLE2LEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTROUTINGTABLE2LEVELMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

//TODO: remove after testing
#include "../../../common/time/TimeUtils.h"

#include <vector>
#include <unordered_map>
#include <boost/functional/hash.hpp>


class ResultRoutingTable2LevelMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<ResultRoutingTable2LevelMessage> Shared;

public:
    ResultRoutingTable2LevelMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID,
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &rt2);

    ResultRoutingTable2LevelMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>>& rt2();

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

protected:
    typedef uint32_t RecordNumber;
    typedef RecordNumber RecordCount;

protected:
    size_t rt2ByteSize() const;

protected:
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> mRT2;
};


#endif //GEO_NETWORK_CLIENT_RESULTROUTINGTABLE2LEVELMESSAGE_H
