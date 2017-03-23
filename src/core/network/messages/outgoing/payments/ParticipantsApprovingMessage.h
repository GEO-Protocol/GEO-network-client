#ifndef GEO_NETWORK_CLIENT_PARTICIPANTSAPPROVINGMESSAGE_H
#define GEO_NETWORK_CLIENT_PARTICIPANTSAPPROVINGMESSAGE_H


#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/exceptions/NotFoundError.h"
#include "../../../../common/exceptions/OverflowError.h"

#include <boost/container/flat_map.hpp>


class ParticipantsApprovingMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<ParticipantsApprovingMessage> Shared;
    typedef shared_ptr<const ParticipantsApprovingMessage> ConstShared;

    enum Vote {
        Approved = 0,
        Rejected = 1,
        Uncertain = 2,
    };

public:
    ParticipantsApprovingMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID);

    ParticipantsApprovingMessage(
        BytesShared buffer);

    void addParticipant(
        const NodeUUID &participant);

    const NodeUUID& nextParticipant(
        const NodeUUID &currentNodeUUID) const;

    const NodeUUID& firstParticipant() const;

    Vote vote(
        const NodeUUID &participant) const;

    void approve(
        const NodeUUID &participant) const;

    void reject(
        const NodeUUID &participant) const;

    bool containsRejectVote() const;

    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes();

protected:
    virtual void deserializeFromBytes(
        BytesShared buffer);

protected:
    /*
     * It is necessary to use flat map here:
     * this container predicts order in which
     * this message would be transamitted between the nodes.
     *
     * In the protocol this approach is described as set of pairs <NodeUUID, vote>,
     * that is sorted in ASC order.
     *
     * Current realisation is simplififed:
     * * set of pairs was replaced by the map;
     * * ascending order is provided by the flat map by default.
     */
    boost::container::flat_map<NodeUUID, Vote> mVotes;
};



#endif //GEO_NETWORK_CLIENT_PARTICIPANTSAPPROVINGMESSAGE_H
