#ifndef GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
#define GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"
#include "../../../../contractors/addresses/BaseAddress.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"


class CyclesThreeNodesBalancesResponseMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<CyclesThreeNodesBalancesResponseMessage> Shared;

public:
    CyclesThreeNodesBalancesResponseMessage(
        const SerializedEquivalent equivalent,
        ContractorID idOnReceiverSide,
        const TransactionUUID &transactionUUID,
        vector<BaseAddress::Shared> &neighbors);

    CyclesThreeNodesBalancesResponseMessage(
        BytesShared buffer);

    virtual pair<BytesShared, size_t> serializeToBytes() const override;

    const MessageType typeID() const
        noexcept;

    vector<BaseAddress::Shared> commonNodes()
        noexcept;

protected:
    vector<BaseAddress::Shared> mNeighbors;
};

#endif //GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
