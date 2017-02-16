#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H

#include "../../../common/Types.h"
#include "../../../settings/Settings.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"
#include "../TransactionMessage.hpp"
#include "../Message.hpp"

class GetTopologyAndBalancesMessageInBetweenNode:
        public TransactionMessage {

public:
    GetTopologyAndBalancesMessageInBetweenNode(
            const TrustLineAmount maxFlow,
            const byte max_depth,
            vector<NodeUUID> &path);
    GetTopologyAndBalancesMessageInBetweenNode(BytesShared buffer);
    typedef uint16_t MessageType;
public:
    typedef shared_ptr<GetTopologyAndBalancesMessageInBetweenNode> Shared;


public:
    const TrustLineUUID &trustLineUUID() const;
    const TransactionUUID &transactionUUID() const;
    const MessageType typeID() const;

protected:
    static const size_t kOffsetToInheritedBytes();
    void deserializeFromBytes(
            BytesShared buffer);

private:
    pair<BytesShared, size_t> serializeToBytes();

private:
    vector<NodeUUID> mPath;
    TrustLineAmount mMaxFlow;
    uint8_t mMax_depth;
};


#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
