#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H

#include "../../../common/Types.h"
#include "../../../settings/Settings.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"
#include "../TransactionMessage.hpp"
#include "../Message.hpp"

class InBetweenNodeTopologyMessage:
        public Message {

public:
    InBetweenNodeTopologyMessage(
            const TrustLineBalance maxFlow,
            const byte max_depth,
            vector<NodeUUID> &path);
    InBetweenNodeTopologyMessage(BytesShared buffer);

public:
    typedef shared_ptr<InBetweenNodeTopologyMessage> Shared;

public:
    const TrustLineUUID &trustLineUUID() const;
    const TransactionUUID &transactionUUID() const;
    const MessageType typeID() const;

public:
    TrustLineBalance getMaxFlow();
    vector<NodeUUID> getPath();
protected:
    static const size_t kOffsetToInheritedBytes();
    void deserializeFromBytes(
            BytesShared buffer);
    pair<BytesShared, size_t> serializeToBytes();

protected:
    vector<NodeUUID> mPath;
    TrustLineBalance mMaxFlow;
    uint8_t mMaxDepth;
    static uint8_t mNodesInPath;
};


#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
