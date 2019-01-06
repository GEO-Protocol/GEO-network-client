#ifndef GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESMESSAGE_H

#include "../Message.hpp"
#include "../../../transactions/transactions/base/TransactionUUID.h"
#include "../../../crypto/lamportscheme.h"

#include <map>

using namespace crypto;

class ObservingParticipantsVotesMessage : public Message {

public:
    typedef shared_ptr<ObservingParticipantsVotesMessage> Shared;

public:
    ObservingParticipantsVotesMessage(
        const TransactionUUID& transactionUUID,
        const map<PaymentNodeID, lamport::Signature::Shared>& participantsSignatures);

    ObservingParticipantsVotesMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    const TransactionUUID& transactionUUID() const;

    const map<PaymentNodeID, lamport::Signature::Shared>& participantsSignatures() const;

private:
    TransactionUUID mTransactionUUID;
    map<PaymentNodeID, lamport::Signature::Shared> mParticipantsSignatures;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESMESSAGE_H
