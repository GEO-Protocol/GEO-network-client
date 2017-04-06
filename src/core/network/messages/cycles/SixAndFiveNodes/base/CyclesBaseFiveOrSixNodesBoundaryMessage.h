#ifndef GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESBOUNDARYMESSAGE_H

#include "CyclesBaseFiveOrSixNodesInBetweenMessage.h"
class CycleBaseFiveOrSixNodesBoundaryMessage: public CycleBaseFiveOrSixNodesInBetweenMessage {
public:
    typedef shared_ptr<CycleBaseFiveOrSixNodesBoundaryMessage> Shared;

public:
    CycleBaseFiveOrSixNodesBoundaryMessage(
        vector<NodeUUID> &path,
        const vector<pair<NodeUUID, TrustLineBalance>> &boundaryNodes);

    CycleBaseFiveOrSixNodesBoundaryMessage(
        BytesShared buffer);

    virtual const MessageType typeID() const = 0;
    const bool isCyclesDiscoveringResponseMessage() const;
    const vector<pair<NodeUUID, TrustLineBalance>> BoundaryNodes() const;


protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);


private:
    vector<pair<NodeUUID, TrustLineBalance>> mBoundaryNodes;
};



#endif //GEO_NETWORK_CLIENT_CYCLEBASEFIVEORSIXNODESBOUNDARYMESSAGE_H
