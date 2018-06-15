#ifndef GEO_NETWORK_CLIENT_TRANSACTIONPUBLICKEYHASHMESSAGE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONPUBLICKEYHASHMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../crypto/lamportscheme.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

using namespace crypto;

class TransactionPublicKeyHashMessage : public TransactionMessage {

public:
    typedef shared_ptr<TransactionPublicKeyHashMessage> Shared;

public:
    TransactionPublicKeyHashMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const PaymentNodeID paymentNodeID,
        const uint32_t transactionPublicKeyHash);

    TransactionPublicKeyHashMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const PaymentNodeID paymentNodeID,
        const uint32_t transactionPublicKeyHash,
        const TrustLineAmount &amount,
        const KeyNumber publicKeyNumber,
        const lamport::Signature::Shared signature);

    TransactionPublicKeyHashMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const PaymentNodeID paymentNodeID() const;

    const uint32_t transactionPublicKeyHash() const;

    bool isReceiptContains() const;

    const TrustLineAmount& amount() const;

    const KeyNumber publicKeyNumber() const;

    const lamport::Signature::Shared signature() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

private:
    PaymentNodeID mPaymentNodeID;
    uint32_t mTransactionPublicKeyHash;
    bool mIsReceiptContains;
    TrustLineAmount mAmount;
    KeyNumber mPublicKeyNumber;
    lamport::Signature::Shared mSignature;
};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONPUBLICKEYHASHMESSAGE_H
