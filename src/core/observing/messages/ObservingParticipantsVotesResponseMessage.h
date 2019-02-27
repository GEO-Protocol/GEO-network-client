#ifndef GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESRESPONSEMESSAGE_H

#include "base/ObservingResponseMessage.h"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../crypto/lamportscheme.h"

#include <map>

using namespace crypto;

class ObservingParticipantsVotesResponseMessage : public ObservingResponseMessage {

public:
    typedef shared_ptr<ObservingParticipantsVotesResponseMessage> Shared;

public:
    ObservingParticipantsVotesResponseMessage(
        BytesShared buffer);

    bool isParticipantsVotesPresent() const;

    const TransactionUUID& transactionUUID() const;

    const BlockNumber maximalClaimingBlockNumber() const;

    const map<PaymentNodeID, lamport::Signature::Shared>& participantsSignatures() const;

private:
    ObservingTransaction::SerializedObservingResponseType mObservingResponse;
    bool mIsParticipantsVotesPresent;
    TransactionUUID mTransactionUUID;
    BlockNumber mMaximalClaimingBlockNumber;
    map<PaymentNodeID, lamport::Signature::Shared> mParticipantsSignatures;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESRESPONSEMESSAGE_H
