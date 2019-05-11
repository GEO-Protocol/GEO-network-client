#ifndef GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H

#include "base/RequestMessageWithReservations.h"
#include "../../../contractors/Contractor.h"
#include "../../../crypto/lamportscheme.h"
#include <map>

using namespace crypto;

class FinalAmountsConfigurationMessage : public RequestMessageWithReservations {

public:
    typedef shared_ptr<FinalAmountsConfigurationMessage> Shared;

public:
    FinalAmountsConfigurationMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
        const map<PaymentNodeID, Contractor::Shared> &paymentParticipants,
        const BlockNumber maximalClaimingBlockNumber);

    // if coordinator has reservation with current node it also send receipt
    FinalAmountsConfigurationMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
        const map<PaymentNodeID, Contractor::Shared> &mPaymentParticipants,
        const BlockNumber maximalClaimingBlockNumber,
        const KeyNumber publicKeyNumber,
        const lamport::Signature::Shared signature,
        const lamport::KeyHash::Shared transactionPublicKeyHash);

    FinalAmountsConfigurationMessage(
        BytesShared buffer);

    const MessageType typeID() const override;

    const map<PaymentNodeID, Contractor::Shared> &paymentParticipants() const;

    const BlockNumber maximalClaimingBlockNumber() const;

    bool isReceiptContains() const;

    const KeyNumber publicKeyNumber() const;

    const lamport::Signature::Shared signature() const;

    const lamport::KeyHash::Shared transactionPublicKeyHash() const;

    pair<BytesShared, size_t> serializeToBytes() const override;

private:
    map<PaymentNodeID, Contractor::Shared> mPaymentParticipants;
    BlockNumber mMaximalClaimingBlockNumber;
    bool mIsReceiptContains;
    KeyNumber mPublicKeyNumber;
    lamport::Signature::Shared mSignature;
    lamport::KeyHash::Shared mTransactionPublicKeyHash;
};


#endif //GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H
