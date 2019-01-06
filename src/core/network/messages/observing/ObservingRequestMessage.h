#ifndef GEO_NETWORK_CLIENT_OBSERVINGREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGREQUESTMESSAGE_H

#include "../Message.hpp"
#include "../../../transactions/transactions/base/TransactionUUID.h"
#include "../../../crypto/lamportkeys.h"

#include <map>

using namespace crypto;

class ObservingRequestMessage : public Message {

public:
    typedef shared_ptr<ObservingRequestMessage> Shared;

public:
    ObservingRequestMessage(
        const TransactionUUID& transactionUUID,
        const map<PaymentNodeID, lamport::PublicKey::Shared>& participantsPublicKeys);

    // todo : this constructor is not neccessary
    ObservingRequestMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const;

private:
    TransactionUUID mTransactionUUID;
    map<PaymentNodeID, lamport::PublicKey::Shared> mParticipantsPublicKeys;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGREQUESTMESSAGE_H
