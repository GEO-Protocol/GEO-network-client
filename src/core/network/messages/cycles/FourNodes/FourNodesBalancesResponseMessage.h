#ifndef GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
#include "../../Message.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"

class FourNodesBalancesResponseMessage: public TransactionMessage {
public:
    typedef shared_ptr<FourNodesBalancesResponseMessage> Shared;
public:
    FourNodesBalancesResponseMessage(
            vector<pair<NodeUUID, TrustLineBalance>> &neighborsBalancesCreditors,
            vector<pair<NodeUUID, TrustLineBalance>> &neighborsBalancesDebtors
    );

    FourNodesBalancesResponseMessage(
            BytesShared buffer);

    const MessageType typeID() const;
    const bool isTransactionMessage() const;
    vector<pair<NodeUUID, TrustLineBalance>> NeighborsBalancesDebtors();
    vector<pair<NodeUUID, TrustLineBalance>> NeighborsBalancesCreditors();
protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
            BytesShared buffer);


protected:
    vector<pair<NodeUUID, TrustLineBalance>> mNeighborsBalancesDebtors;
    vector<pair<NodeUUID, TrustLineBalance>> mNeighborsBalancesCreditors;
    TrustLineBalance mMaxFlow;
};
#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
