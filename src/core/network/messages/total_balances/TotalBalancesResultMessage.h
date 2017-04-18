#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESRESULTMESSAGE_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESRESULTMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

class TotalBalancesResultMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<TotalBalancesResultMessage> Shared;

public:
    TotalBalancesResultMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &totalIncomingTrust,
        const TrustLineAmount &totalTrustUsedByContractor,
        const TrustLineAmount &totalOutgoingTrust,
        const TrustLineAmount &totalTrustUsedBySelf);

    TotalBalancesResultMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    const TrustLineAmount& totalIncomingTrust() const;

    const TrustLineAmount& totalTrustUsedByContractor() const;

    const TrustLineAmount& totalOutgoingTrust() const;

    const TrustLineAmount& totalTrustUsedBySelf() const;

private:
    TrustLineAmount mTotalIncomingTrust;
    TrustLineAmount mTotalTrustUsedByContractor;
    TrustLineAmount mTotalOutgoingTrust;
    TrustLineAmount mTotalTrustUsedBySelf;
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESRESULTMESSAGE_H
