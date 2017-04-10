#ifndef GEO_NETWORK_CLIENT_RESULTROUTINGTABLE3LEVELVECOTRMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTROUTINGTABLE3LEVELVECOTRMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"
//TODO: remove after testing
#include "../../../common/time/TimeUtils.h"

#include <vector>

class ResultRoutingTable3LevelVectorMessage : public TransactionMessage {

public:
    typedef shared_ptr<ResultRoutingTable3LevelVectorMessage> Shared;

public:

    ResultRoutingTable3LevelVectorMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID,
        vector<pair<NodeUUID, NodeUUID>> &rt3);

    ResultRoutingTable3LevelVectorMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    vector<pair<NodeUUID, NodeUUID>>& rt3();

    pair<BytesShared, size_t> serializeToBytes();

private:

    typedef uint32_t RecordNumber;
    typedef RecordNumber RecordCount;

private:

    size_t rt3ByteSize();

    void deserializeFromBytes(
        BytesShared buffer);

private:

    vector<pair<NodeUUID, NodeUUID>> mRT3;

};


#endif //GEO_NETWORK_CLIENT_RESULTROUTINGTABLE3LEVELVECOTRMESSAGE_H
