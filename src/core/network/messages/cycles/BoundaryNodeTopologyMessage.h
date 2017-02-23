#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H

//TODO:: (D.V.) Of course I understand that all imports already included in parent class,
//TODO:: but in OOP, every class is independent and self-sufficient entity, so please,
//TODO:: include here all dependencies that this class will be using.
#include "InBetweenNodeTopologyMessage.h"

class BoundaryNodeTopologyMessage: public InBetweenNodeTopologyMessage {
public:
    typedef shared_ptr<BoundaryNodeTopologyMessage> Shared;

public:
    BoundaryNodeTopologyMessage(
            const TrustLineBalance maxFlow, //TODO:: (D.V.) TrustLineBalance is non primitive type, use address, don't copies tmp value in constructor anymore.
            const byte max_depth,
            vector<NodeUUID> &path, //TODO:: (D.V.) (Recommendation) Try to use move semantic.
            const vector<pair<NodeUUID, TrustLineBalance>> boundaryNodes); //TODO:: Seriously, copy??

    // TODO:: Constructor's or function's parameters starts from new line
    BoundaryNodeTopologyMessage(
        BytesShared buffer);

    // TODO:: (D.V.) If this class is a last inherit, change access modifiers to private.
protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
            BytesShared buffer);

    //TODO:: (D.V.) Separate members from methods!
private:
    vector<pair<NodeUUID, TrustLineBalance>> mBoundaryNodes;
};

#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H
