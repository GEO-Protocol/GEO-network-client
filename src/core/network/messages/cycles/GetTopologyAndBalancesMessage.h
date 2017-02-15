#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H

#include "../../../common/Types.h"
#include "../../../settings/Settings.h"
#include "../Message.hpp"

class GetTopologyAndBalancesMessage: public Message {

public:
    GetTopologyAndBalancesMessage(const TrustLineAmount maxFlow, const byte max_depth);
    GetTopologyAndBalancesMessage(BytesShared buffer);
public:
    typedef shared_ptr<GetTopologyAndBalancesMessage> Shared;

private:
    void deserializeFromBytes(
            BytesShared buffer);

    void serializedToBytes();

private:
    vector<NodeUUID> mPath;
    TrustLineAmount mMaxFlow;
    byte mMax_depth;
    vector<pair<NodeUUID, TrustLineAmount>> mBoundary_nodes;
};


#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
