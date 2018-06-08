#ifndef GEO_NETWORK_CLIENT_PARTICIPANTVOTEMESSAGE_H
#define GEO_NETWORK_CLIENT_PARTICIPANTVOTEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class ParticipantVoteMessage : public TransactionMessage {

public:
    typedef shared_ptr<ParticipantVoteMessage> Shared;

public:
    ParticipantVoteMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        size_t signBytesCount,
        BytesShared sign);

    ParticipantVoteMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const pair<BytesShared, size_t> sign() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

private:
    BytesShared mSign;
    size_t mSignBytesCount;
};


#endif //GEO_NETWORK_CLIENT_PARTICIPANTVOTEMESSAGE_H
