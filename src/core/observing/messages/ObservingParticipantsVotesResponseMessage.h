#ifndef GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESRESPONSEMESSAGE_H

#include "ObservingMessage.hpp"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../crypto/lamportscheme.h"

#include <map>

using namespace crypto;

class ObservingParticipantsVotesResponseMessage : public ObservingMessage {

public:
    typedef shared_ptr<ObservingParticipantsVotesResponseMessage> Shared;

public:
    ObservingParticipantsVotesResponseMessage(
        BytesShared buffer);

    const MessageType typeID() const override;

    const TransactionUUID& transactionUUID() const;

    const map<PaymentNodeID, lamport::Signature::Shared>& participantsSignatures() const;

private:
    TransactionUUID mTransactionUUID;
    map<PaymentNodeID, lamport::Signature::Shared> mParticipantsSignatures;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESRESPONSEMESSAGE_H
