#ifndef GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESAPPENDREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESAPPENDREQUESTMESSAGE_H

#include "base/ObservingMessage.hpp"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../crypto/lamportscheme.h"

#include <map>

using namespace crypto;

class ObservingParticipantsVotesAppendRequestMessage : public ObservingMessage {

public:
    typedef shared_ptr<ObservingParticipantsVotesAppendRequestMessage> Shared;

public:
    ObservingParticipantsVotesAppendRequestMessage(
        const TransactionUUID& transactionUUID,
        BlockNumber maximalClaimingBlockNumber,
        map<PaymentNodeID, lamport::Signature::Shared> participantsSignatures);

    const MessageType typeID() const override;

    BytesShared serializeToBytes() const override;

    size_t serializedSize() const override;

private:
    TransactionUUID mTransactionUUID;
    BlockNumber mMaximalClaimingBlockNumber;
    map<PaymentNodeID, lamport::Signature::Shared> mParticipantsSignatures;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESAPPENDREQUESTMESSAGE_H
