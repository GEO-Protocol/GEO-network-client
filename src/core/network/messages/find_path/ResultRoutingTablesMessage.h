#ifndef GEO_NETWORK_CLIENT_RESULTROUTINGTABLESMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTROUTINGTABLESMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

#include <vector>
#include <unordered_map>

class ResultRoutingTablesMessage : public TransactionMessage {

public:
    typedef shared_ptr<ResultRoutingTablesMessage> Shared;

public:

    ResultRoutingTablesMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID,
        vector<NodeUUID> &rt1,
        unordered_map<NodeUUID, vector<NodeUUID>> &rt2,
        unordered_map<NodeUUID, vector<NodeUUID>> &rt3);

    ResultRoutingTablesMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    vector<NodeUUID>& rt1();

    unordered_map<NodeUUID, vector<NodeUUID>>& rt2();

    unordered_map<NodeUUID, vector<NodeUUID>>& rt3();

    pair<BytesShared, size_t> serializeToBytes();

private:

    typedef uint32_t RecordNumber;
    typedef RecordNumber RecordCount;

private:

    size_t rt2ByteSize();

    size_t rt3ByteSize();

    void deserializeFromBytes(
            BytesShared buffer);

private:

    vector<NodeUUID> mRT1;
    unordered_map<NodeUUID, vector<NodeUUID>> mRT2;
    unordered_map<NodeUUID, vector<NodeUUID>> mRT3;

};


#endif //GEO_NETWORK_CLIENT_RESULTROUTINGTABLESMESSAGE_H
