#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H

#include "../../../common/Types.h"
#include "../../../settings/Settings.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"
#include "../Message.hpp"

class GetTopologyAndBalancesMessageFirstLevelNode: public Message {

public:
    GetTopologyAndBalancesMessageFirstLevelNode(
            const TrustLineAmount maxFlow,
            const byte max_depth,
            vector<NodeUUID> &path);
    GetTopologyAndBalancesMessageFirstLevelNode(BytesShared buffer);
public:
    typedef shared_ptr<GetTopologyAndBalancesMessageFirstLevelNode> Shared;


public:
    const TrustLineUUID &trustLineUUID() const {
        throw NotImplementedError("TrustLinesMessage: public Message::trustLineUUID:"
                                          "Method not implemented.");
    }
    const TransactionUUID &transactionUUID() const {
        throw NotImplementedError("TrustLinesMessage: public Message::trustLineUUID:"
                                          "Method not implemented.");
    }
    const MessageType typeID() const{
        throw NotImplementedError("TrustLinesMessage: public Message::trustLineUUID:"
                                          "Method not implemented.");
    }

private:
    void deserializeFromBytes(
            BytesShared buffer);

    pair<BytesShared, size_t> serializedToBytes();

private:
    vector<NodeUUID> mPath;
    TrustLineAmount mMaxFlow;
    byte mMax_depth;
//    vector<pair<NodeUUID, TrustLineAmount>> mBoundary_nodes;
//    const typeID();
//    const transactionUUID;

};


#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESMESSAGE_H
