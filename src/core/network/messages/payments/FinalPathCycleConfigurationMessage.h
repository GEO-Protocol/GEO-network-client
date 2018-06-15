#ifndef GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H

#include "base/RequestCycleMessage.h"
#include "../../../crypto/lamportscheme.h"
#include <map>

using namespace crypto;

class FinalPathCycleConfigurationMessage :
    public RequestCycleMessage {

public:
    typedef shared_ptr<FinalPathCycleConfigurationMessage> Shared;

public:
    FinalPathCycleConfigurationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount,
        const map<NodeUUID, PaymentNodeID> &paymentNodesIds);

    FinalPathCycleConfigurationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount,
        const map<NodeUUID, PaymentNodeID> &paymentNodesIds,
        const KeyNumber publicKeyNumber,
        const lamport::Signature::Shared signature);

    FinalPathCycleConfigurationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const map<NodeUUID, PaymentNodeID>& paymentNodesIds() const;

    bool isReceiptContains() const;

    const KeyNumber publicKeyNumber() const;

    const lamport::Signature::Shared signature() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

private:
    map<NodeUUID, PaymentNodeID> mPaymentNodesIds;
    bool mIsReceiptContains;
    KeyNumber mPublicKeyNumber;
    lamport::Signature::Shared mSignature;
};


#endif //GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H
