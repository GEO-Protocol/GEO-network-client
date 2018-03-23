#ifndef GEO_NETWORK_CLIENT_FOURNODESNEGATIVEBALANCEREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESNEGATIVEBALANCEREQUESTMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include <vector>


class CyclesFourNodesNegativeBalanceRequestMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<CyclesFourNodesNegativeBalanceRequestMessage> Shared;

public:
    CyclesFourNodesNegativeBalanceRequestMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const NodeUUID &contractor,
        vector<NodeUUID> &checkedNodes);

    CyclesFourNodesNegativeBalanceRequestMessage(
        BytesShared buffer);

    virtual pair<BytesShared, size_t> serializeToBytes()const
        throw(bad_alloc);

    const MessageType typeID() const;

    vector<NodeUUID> checkedNodes() const;

    const NodeUUID contractor() const;

protected:
    vector<NodeUUID> mCheckedNodes;
    NodeUUID mContractorUUID;
};

#endif //GEO_NETWORK_CLIENT_FOURNODESNEGATIVEBALANCEREQUESTMESSAGE_H
