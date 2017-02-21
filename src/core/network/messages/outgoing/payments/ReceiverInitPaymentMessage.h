#ifndef GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTMESSAGE_H

#include "../../TransactionMessage.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"


class ReceiverInitPaymentMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<ReceiverInitPaymentMessage> Shared;
    typedef shared_ptr<const ReceiverInitPaymentMessage> ConstShared;

public:
    ReceiverInitPaymentMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &totalPaymentAmount);

    ReceiverInitPaymentMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

protected:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    mutable TrustLineAmount mTotalPaymentAmount;
};


#endif //GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTMESSAGE_H
