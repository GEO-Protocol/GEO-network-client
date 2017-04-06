#ifndef GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESBOUNDARYMESSAGE_H

#include "CyclesBaseFiveOrSixNodesInBetweenMessage.h"
class CyclesBaseFiveOrSixNodesBoundaryMessage:
        public CycleBaseFiveOrSixNodesInBetweenMessage {
public:
    typedef shared_ptr<CyclesBaseFiveOrSixNodesBoundaryMessage> Shared;

public:
    CyclesBaseFiveOrSixNodesBoundaryMessage(
        vector<NodeUUID> &path,
        vector<pair<NodeUUID, TrustLineBalance>> &boundaryNodes);

    CyclesBaseFiveOrSixNodesBoundaryMessage(
        BytesShared buffer);

public:
    pair<BytesShared, size_t> serializeToBytes();

    virtual const MessageType typeID() const = 0;

    const bool isCyclesDiscoveringResponseMessage() const;

    const vector<pair<NodeUUID, TrustLineBalance>> BoundaryNodes() const;

protected:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    vector<pair<NodeUUID, TrustLineBalance>> mBoundaryNodes;
};



#endif //GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESBOUNDARYMESSAGE_H
