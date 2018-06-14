#ifndef GEO_NETWORK_CLIENT_PARTICIPANTVOTEMESSAGE_H
#define GEO_NETWORK_CLIENT_PARTICIPANTVOTEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../crypto/lamportscheme.h"

using namespace crypto;

class ParticipantVoteMessage : public TransactionMessage {

public:
    typedef shared_ptr<ParticipantVoteMessage> Shared;

public:
    ParticipantVoteMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        lamport::Signature::Shared sign);

    ParticipantVoteMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const lamport::Signature::Shared sign() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

private:
    lamport::Signature::Shared mSign;
};


#endif //GEO_NETWORK_CLIENT_PARTICIPANTVOTEMESSAGE_H
