#ifndef GEO_NETWORK_CLIENT_OBSERVINGREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGREQUESTMESSAGE_H

#include "base/ObservingMessage.hpp"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../crypto/lamportkeys.h"

#include <map>

using namespace crypto;

class ObservingClaimAppendRequestMessage : public ObservingMessage {

public:
    typedef shared_ptr<ObservingClaimAppendRequestMessage> Shared;

public:
    ObservingClaimAppendRequestMessage(
        const TransactionUUID& transactionUUID,
        BlockNumber maximalClaimingBlockNumber,
        const map<PaymentNodeID, lamport::PublicKey::Shared>& participantsPublicKeys);

    const TransactionUUID& transactionUUID() const;

    const BlockNumber maximalClaimingBlockNumber() const;

    const MessageType typeID() const override;

    BytesShared serializeToBytes() const override;

    size_t serializedSize() const override;

private:
    TransactionUUID mTransactionUUID;
    BlockNumber mMaximalClaimingBlockNumber;
    map<PaymentNodeID, lamport::PublicKey::Shared> mParticipantsPublicKeys;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGREQUESTMESSAGE_H
