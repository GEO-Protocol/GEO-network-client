#ifndef GEO_NETWORK_CLIENT_PARTICIPANTSAPPROVINGMESSAGE_H
#define GEO_NETWORK_CLIENT_PARTICIPANTSAPPROVINGMESSAGE_H


#include "../base/transaction/TransactionMessage.h"

#include "../../../common/exceptions/NotFoundError.h"
#include "../../../common/exceptions/OverflowError.h"

#include <boost/container/flat_map.hpp>
#include <map>

/**
 * This message is used to achieve consensus between transaction participants.
 * It contains UUIDs of all nodes that are involved into the operation and their votes.
 *
 * TODO: [mvp+] add participants signing
 */
class ParticipantsVotesMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<ParticipantsVotesMessage> Shared;

public:
    ParticipantsVotesMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        map<PaymentNodeID, BytesShared> &participantsSigns);

    ParticipantsVotesMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const map<PaymentNodeID, BytesShared>& participantsSigns() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

private:
    map<PaymentNodeID, BytesShared> mParticipantsSigns;
};
#endif //GEO_NETWORK_CLIENT_PARTICIPANTSAPPROVINGMESSAGE_H
