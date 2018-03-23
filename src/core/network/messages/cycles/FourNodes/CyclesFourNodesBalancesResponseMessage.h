#ifndef GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"


class CyclesFourNodesBalancesResponseMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<CyclesFourNodesBalancesResponseMessage> Shared;

public:
    CyclesFourNodesBalancesResponseMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        vector<NodeUUID> &suitableNodes);

    CyclesFourNodesBalancesResponseMessage(
        BytesShared buffer);

    virtual pair<BytesShared, size_t> serializeToBytes()const
        throw(bad_alloc);

    const MessageType typeID() const;

    vector<NodeUUID> suitableNodes() const;

protected:
    vector<NodeUUID> mSuitableNodes;
};
#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
