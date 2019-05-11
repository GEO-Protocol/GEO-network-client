#ifndef GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H

#include "../../SenderMessage.h"

#include "../../../../transactions/transactions/base/TransactionUUID.h"


class TransactionMessage:
    public SenderMessage {

public:
    typedef shared_ptr<TransactionMessage> Shared;
    typedef shared_ptr<const TransactionMessage> ConstShared;

public:
    TransactionMessage(
        const SerializedEquivalent equivalent,
        const TransactionUUID &transactionUUID);

    TransactionMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> &senderAddresses,
        const TransactionUUID &transactionUUID);

    TransactionMessage(
        const SerializedEquivalent equivalent,
        ContractorID idOnReceiverSide,
        const TransactionUUID &transactionUUID);

    TransactionMessage(
        BytesShared buffer);

    pair<BytesShared, size_t> serializeToBytes() const override;

    const TransactionUUID &transactionUUID() const;

protected:
    const size_t kOffsetToInheritedBytes() const override;

    const bool isTransactionMessage() const override;

protected:
    const TransactionUUID mTransactionUUID;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H
