#ifndef GEO_NETWORK_CLIENT_RESERVEBALANCEREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_RESERVEBALANCEREQUESTMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"


class ReserveBalanceRequestMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<ReserveBalanceRequestMessage> Shared;
    typedef shared_ptr<const ReserveBalanceRequestMessage> ConstShared;

public:
    ReserveBalanceRequestMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount,
        const NodeUUID &nextNodeInThePath);

    ReserveBalanceRequestMessage(
        BytesShared buffer);

    const TrustLineAmount& amount() const;
    const NodeUUID& nextNodeInThePathUUID() const;

private:
    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    TrustLineAmount mAmount;
    NodeUUID mNextNodeInPathUUID;
};

#endif // GEO_NETWORK_CLIENT_RESERVEBALANCEREQUESTMESSAGE_H
