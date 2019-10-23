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
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID &transactionUUID,
        BaseAddress::Shared contractorAddress,
        vector<BaseAddress::Shared> checkedNodes);

    CyclesFourNodesNegativeBalanceRequestMessage(
        BytesShared buffer);

    virtual pair<BytesShared, size_t> serializeToBytes()const override;

    const MessageType typeID() const override;

    vector<BaseAddress::Shared> checkedNodes() const;

    BaseAddress::Shared contractorAddress() const;

protected:
    vector<BaseAddress::Shared> mCheckedNodes;
    BaseAddress::Shared mContractorAddress;
};

#endif //GEO_NETWORK_CLIENT_FOURNODESNEGATIVEBALANCEREQUESTMESSAGE_H
