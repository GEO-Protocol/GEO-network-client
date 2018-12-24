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
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID &transactionUUID,
        vector<BaseAddress::Shared> &suitableNodes);

    CyclesFourNodesBalancesResponseMessage(
        BytesShared buffer);

    virtual pair<BytesShared, size_t> serializeToBytes()const
        throw(bad_alloc);

    const MessageType typeID() const;

    vector<BaseAddress::Shared> suitableNodes() const;

protected:
    vector<BaseAddress::Shared> mSuitableNodes;
};
#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
