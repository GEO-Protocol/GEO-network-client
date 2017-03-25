#ifndef GEO_NETWORK_CLIENT_RESULTROUTINGTABLESMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTROUTINGTABLESMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

#include <vector>
#include <tuple>

class ResultRoutingTablesMessage : public TransactionMessage {

public:
    typedef shared_ptr<ResultRoutingTablesMessage> Shared;

public:

    ResultRoutingTablesMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID,
        vector<pair<const NodeUUID, const TrustLineDirection>> rt1,
        vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> rt2,
        vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> rt3);

    ResultRoutingTablesMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    vector<pair<const NodeUUID, const TrustLineDirection>> rt1();

    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> rt2();

    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> rt3();

    pair<BytesShared, size_t> serializeToBytes();

private:

    void deserializeFromBytes(
            BytesShared buffer);

private:

    vector<pair<const NodeUUID, const TrustLineDirection>> mRT1;
    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> mRT2;
    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> mRT3;

};


#endif //GEO_NETWORK_CLIENT_RESULTROUTINGTABLESMESSAGE_H
