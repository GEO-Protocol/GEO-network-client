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
        vector<BaseAddress::Shared> &path,
        vector<BaseAddress::Shared> &boundaryNodes);

    CyclesBaseFiveOrSixNodesBoundaryMessage(
        BytesShared buffer);

public:
    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

    vector<BaseAddress::Shared> BoundaryNodes() const;

private:
    vector<BaseAddress::Shared> mBoundaryNodes;
};



#endif //GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESBOUNDARYMESSAGE_H
