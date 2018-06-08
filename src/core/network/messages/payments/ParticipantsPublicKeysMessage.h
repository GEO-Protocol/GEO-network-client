#ifndef GEO_NETWORK_CLIENT_PARTICIPANTSPUBLICKEYSMESSAGE_H
#define GEO_NETWORK_CLIENT_PARTICIPANTSPUBLICKEYSMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../crypto/CryptoKey.h"

#include <map>

class ParticipantsPublicKeysMessage : public TransactionMessage {

public:
    typedef shared_ptr<ParticipantsPublicKeysMessage> Shared;

public:
    ParticipantsPublicKeysMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const map<PaymentNodeID, CryptoKey>& publicKeys);

    ParticipantsPublicKeysMessage(
            BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const map<PaymentNodeID, CryptoKey>& publicKeys() const;

private:
    map<PaymentNodeID, CryptoKey> mPublicKeys;
};


#endif //GEO_NETWORK_CLIENT_PARTICIPANTSPUBLICKEYSMESSAGE_H
