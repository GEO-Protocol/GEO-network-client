#ifndef GEO_NETWORK_CLIENT_PARTICIPANTVOTEMESSAGE_H
#define GEO_NETWORK_CLIENT_PARTICIPANTVOTEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../crypto/lamportscheme.h"

using namespace crypto;

class ParticipantVoteMessage : public TransactionMessage {

public:
    typedef shared_ptr<ParticipantVoteMessage> Shared;

    enum OperationState {
        Accepted = 1,
        Rejected = 2,
    };

public:
    ParticipantVoteMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> &senderAddresses,
        const TransactionUUID &transactionUUID,
        lamport::Signature::Shared signature = nullptr);

    ParticipantVoteMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const OperationState state() const;

    const lamport::Signature::Shared signature() const;

    pair<BytesShared, size_t> serializeToBytes() const override;

private:
    typedef byte SerializedOperationState;

private:
    OperationState mState;
    lamport::Signature::Shared mSignature;
};


#endif //GEO_NETWORK_CLIENT_PARTICIPANTVOTEMESSAGE_H
