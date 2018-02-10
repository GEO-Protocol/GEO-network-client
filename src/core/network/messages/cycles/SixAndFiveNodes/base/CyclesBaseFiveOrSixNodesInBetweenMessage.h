#ifndef GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESINBETWEENMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESINBETWEENMESSAGE_H

#include "../../../Message.hpp"

#include "../../../../../common/Types.h"
#include "../../../../../common/memory/MemoryUtils.h"
#include "../../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../../../../common/NodeUUID.h"

class CycleBaseFiveOrSixNodesInBetweenMessage:
        public Message {
public:
    typedef shared_ptr<CycleBaseFiveOrSixNodesInBetweenMessage> Shared;
public:
    CycleBaseFiveOrSixNodesInBetweenMessage();
    CycleBaseFiveOrSixNodesInBetweenMessage(
        vector<NodeUUID> &path);
    CycleBaseFiveOrSixNodesInBetweenMessage(
        BytesShared buffer);

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const vector<NodeUUID> Path() const;

    void addNodeToPath(NodeUUID InBetweenNode);

protected:
    void deserializeFromBytes(
        BytesShared buffer);

    const size_t kOffsetToInheritedBytes();

protected:
    vector<NodeUUID> mPath;
};


#endif //GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESINBETWEENMESSAGE_H
