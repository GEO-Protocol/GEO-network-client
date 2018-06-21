#ifndef GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H

#include "base/RequestMessageWithReservations.h"
#include "../../../crypto/lamportscheme.h"
#include <map>

using namespace crypto;

class FinalAmountsConfigurationMessage : public RequestMessageWithReservations {

public:
    typedef shared_ptr<FinalAmountsConfigurationMessage> Shared;

public:
    FinalAmountsConfigurationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
        const map<NodeUUID, PaymentNodeID> &paymentNodesIds);

    // if coordinator has reservation with current node it also send receipt
    FinalAmountsConfigurationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
        const map<NodeUUID, PaymentNodeID> &paymentNodesIds,
        const KeyNumber publicKeyNumber,
        const lamport::Signature::Shared signature);

    FinalAmountsConfigurationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const map<NodeUUID, PaymentNodeID> &paymentNodesIds() const;

    bool isReceiptContains() const;

    const KeyNumber publicKeyNumber() const;

    const lamport::Signature::Shared signature() const;

protected:
    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

private:
    map<NodeUUID, PaymentNodeID> mPaymentNodesIds;
    bool mIsReceiptContains;
    KeyNumber mPublicKeyNumber;
    lamport::Signature::Shared mSignature;
};


#endif //GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H
