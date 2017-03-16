#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H

#include "../Message.hpp"

#include "../../../common/Types.h"
#include "../../../common/memory/MemoryUtils.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

#include "../base/transaction/TransactionMessage.h"
class InBetweenNodeTopologyMessage: public Message {
public:
    typedef shared_ptr<InBetweenNodeTopologyMessage> Shared;
    typedef uint8_t CycleType;
public:
    InBetweenNodeTopologyMessage();
    InBetweenNodeTopologyMessage(
            const CycleType cycleType,
            const TrustLineBalance& maxFlow,
            const byte max_depth,
            vector<NodeUUID> &path);
    InBetweenNodeTopologyMessage(
            BytesShared buffer);

    enum CycleTypeID {
        CycleForSixNodes=1,
        CycleForFiveNodes=2
    };

    NodeUUID &getLastUUIDFromPath();

    const MessageType typeID() const;

    const TrustLineBalance& maxFlow() const;

    const vector<NodeUUID> Path() const;

    const uint8_t maxDepth() const;

    const CycleType cycleType() const;

protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
            BytesShared buffer);

    static const size_t kOffsetToInheritedBytes();

protected:
    vector<NodeUUID> mPath;
    TrustLineBalance mMaxFlow;
    uint8_t mMaxDepth;
    static uint8_t mNodesInPath;
    CycleType mCycleType;
};


#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
