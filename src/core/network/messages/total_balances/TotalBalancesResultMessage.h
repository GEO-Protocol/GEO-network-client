#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESRESULTMESSAGE_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESRESULTMESSAGE_H

#import "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

class TotalBalancesResultMessage : public TransactionMessage {

public:
    typedef shared_ptr<TotalBalancesResultMessage> Shared;

public:

    TotalBalancesResultMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &totalIncomingTrust,
        const TrustLineAmount &totalIncomingTrustUsed,
        const TrustLineAmount &totalOutgoingTrust,
        const TrustLineAmount &totalOutgoingTrustUsed);

    TotalBalancesResultMessage(
            BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    const bool isTotalBalancesResponseMessage() const;

    const TrustLineAmount& totalIncomingTrust() const;

    const TrustLineAmount& totalIncomingTrustUsed() const;

    const TrustLineAmount& totalOutgoingTrust() const;

    const TrustLineAmount& totalOutgoingTrustUsed() const;

private:
    void deserializeFromBytes(
            BytesShared buffer);

private:
    TrustLineAmount mTotalIncomingTrust;
    TrustLineAmount mTotalIncomingTrustUsed;
    TrustLineAmount mTotalOutgoingTrust;
    TrustLineAmount mTotalOutgoingTrustUsed;

};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESRESULTMESSAGE_H
