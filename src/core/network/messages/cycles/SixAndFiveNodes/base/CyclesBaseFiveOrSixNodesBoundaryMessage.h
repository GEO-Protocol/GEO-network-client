#ifndef GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESBOUNDARYMESSAGE_H

#include "CyclesBaseFiveOrSixNodesInBetweenMessage.h"
class CyclesBaseFiveOrSixNodesBoundaryMessage:
        public CycleBaseFiveOrSixNodesInBetweenMessage {
public:
    typedef shared_ptr<CyclesBaseFiveOrSixNodesBoundaryMessage> Shared;

public:
    CyclesBaseFiveOrSixNodesBoundaryMessage(
        const SerializedEquivalent equivalent,
        vector<NodeUUID> &path,
        vector<NodeUUID> &boundaryNodes);

    CyclesBaseFiveOrSixNodesBoundaryMessage(
        BytesShared buffer);

public:
    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

    const vector<NodeUUID> BoundaryNodes() const;

private:
    vector<NodeUUID> mBoundaryNodes;
};



#endif //GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESBOUNDARYMESSAGE_H
