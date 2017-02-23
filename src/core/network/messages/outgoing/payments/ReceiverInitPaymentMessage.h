#ifndef GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include <memory>
#include <utility>
#include <stdint.h>

class ReceiverInitPaymentMessage: public TransactionMessage {

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

private:
    const MessageType typeID() const;
    TrustLineAmount& amount() const;
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    mutable TrustLineAmount mTotalPaymentAmount;
};
#endif //GEO_NETWORK_CLIENT_RECEIVERINITPAYMENTMESSAGE_H
