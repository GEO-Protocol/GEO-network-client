#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H

#include "../../../common/Types.h"
#include "../../../settings/Settings.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"
#include "../Message.hpp"

class GetTopologyAndBalancesMessageFirstLevelNode: public Message {

public:
    GetTopologyAndBalancesMessageFirstLevelNode(
            const TrustLineAmount maxFlow,
            const byte max_depth,
            vector<NodeUUID> &path);
    GetTopologyAndBalancesMessageFirstLevelNode(BytesShared buffer);
public:
    typedef shared_ptr<GetTopologyAndBalancesMessageFirstLevelNode> Shared;


public:
    const TrustLineUUID &trustLineUUID() const;
    const TransactionUUID &transactionUUID() const;
    const MessageType typeID() const;

private:
    void deserializeFromBytes(
            BytesShared buffer);

    pair<BytesShared, size_t> serializeToBytes();

private:
    vector<NodeUUID> mPath;
    TrustLineAmount mMaxFlow;
    byte mMax_depth;
};


#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
