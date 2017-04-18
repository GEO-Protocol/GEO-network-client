#ifndef GEO_NETWORK_CLIENT_RESULTROUTINGTABLE3LEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTROUTINGTABLE3LEVELMESSAGE_H


#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"
//TODO: remove after testing
#include "../../../common/time/TimeUtils.h"

#include <vector>
#include <unordered_map>
#include <boost/functional/hash.hpp>

class ResultRoutingTable3LevelMessage : public TransactionMessage {

public:
    typedef shared_ptr<ResultRoutingTable3LevelMessage> Shared;

public:

    ResultRoutingTable3LevelMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID,
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &rt3);

    ResultRoutingTable3LevelMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>>& rt3();

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

private:
    typedef uint32_t RecordNumber;
    typedef RecordNumber RecordCount;

private:
    size_t rt3ByteSize() const;

private:

    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> mRT3;

};


#endif //GEO_NETWORK_CLIENT_RESULTROUTINGTABLE3LEVELMESSAGE_H
