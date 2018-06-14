#ifndef GEO_NETWORK_CLIENT_PARTICIPANTSPUBLICKEYSMESSAGE_H
#define GEO_NETWORK_CLIENT_PARTICIPANTSPUBLICKEYSMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../crypto/lamportkeys.h"

#include <map>

using namespace crypto;

class ParticipantsPublicKeysMessage : public TransactionMessage {

public:
    typedef shared_ptr<ParticipantsPublicKeysMessage> Shared;

public:
    ParticipantsPublicKeysMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const map<PaymentNodeID, lamport::PublicKey::Shared>& publicKeys);

    ParticipantsPublicKeysMessage(
            BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const map<PaymentNodeID, lamport::PublicKey::Shared>& publicKeys() const;

private:
    map<PaymentNodeID, lamport::PublicKey::Shared> mPublicKeys;
};


#endif //GEO_NETWORK_CLIENT_PARTICIPANTSPUBLICKEYSMESSAGE_H
