#ifndef GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESREQUESTMESSAGE_H

#include "base/ObservingMessage.hpp"
#include "../../transactions/transactions/base/TransactionUUID.h"

class ObservingParticipantsVotesRequestMessage : public ObservingMessage {

public:
    typedef shared_ptr<ObservingParticipantsVotesRequestMessage> Shared;

public:
    ObservingParticipantsVotesRequestMessage(
        const TransactionUUID& transactionUUID,
        BlockNumber maximalClaimingBlockNumber);

    const MessageType typeID() const override;

    BytesShared serializeToBytes() const override;

    size_t serializedSize() const override;

private:
    TransactionUUID mTransactionUUID;
    BlockNumber mMaximalClaimingBlockNumber;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGPARTICIPANTSVOTESREQUESTMESSAGE_H
