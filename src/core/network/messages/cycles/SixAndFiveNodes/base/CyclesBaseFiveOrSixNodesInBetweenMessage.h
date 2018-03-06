#ifndef GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESINBETWEENMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESINBETWEENMESSAGE_H

#include "../../../EquivalentMessage.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/memory/MemoryUtils.h"
#include "../../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../../../../common/NodeUUID.h"

class CycleBaseFiveOrSixNodesInBetweenMessage:
        public EquivalentMessage {
public:
    typedef shared_ptr<CycleBaseFiveOrSixNodesInBetweenMessage> Shared;
public:
    CycleBaseFiveOrSixNodesInBetweenMessage(
        const SerializedEquivalent equivalent,
        vector<NodeUUID> &path);

    CycleBaseFiveOrSixNodesInBetweenMessage(
        BytesShared buffer);

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const vector<NodeUUID> Path() const;

    void addNodeToPath(
        const NodeUUID &inBetweenNode);

protected:
    const size_t kOffsetToInheritedBytes();

protected:
    vector<NodeUUID> mPath;
};


#endif //GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESINBETWEENMESSAGE_H
