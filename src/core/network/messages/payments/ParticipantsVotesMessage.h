#ifndef GEO_NETWORK_CLIENT_PARTICIPANTSAPPROVINGMESSAGE_H
#define GEO_NETWORK_CLIENT_PARTICIPANTSAPPROVINGMESSAGE_H


#include "../base/transaction/TransactionMessage.h"

#include "../../../common/exceptions/NotFoundError.h"
#include "../../../common/exceptions/OverflowError.h"

#include <boost/container/flat_map.hpp>

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
    typedef shared_ptr<const ParticipantsVotesMessage> ConstShared;

    enum Vote {
        Approved = 0,
        Rejected = 1,
        Uncertain = 2,
    };

public:
    ParticipantsVotesMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const NodeUUID &coordinatorUUID);

    ParticipantsVotesMessage(
        BytesShared buffer);

    ParticipantsVotesMessage(
        const NodeUUID &senderUUID,
        const ParticipantsVotesMessage::Shared &message
    );

    void addParticipant(
        const NodeUUID &participant);

    const NodeUUID& firstParticipant() const;

    const NodeUUID& nextParticipant(
        const NodeUUID &currentNodeUUID) const;

    const NodeUUID& coordinatorUUID() const;

    size_t participantsCount () const;

    Vote vote(
        const NodeUUID &participant) const;

    void approve(
        const NodeUUID &participant);

    void reject(
        const NodeUUID &participant);

    bool containsRejectVote() const;

    bool achievedConsensus() const;

    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

#ifdef DEBUG
    const boost::container::flat_map<NodeUUID, Vote>& votes() const;
#endif

protected:
    typedef uint32_t RecordsCount;
    typedef byte SerializedVote;

    virtual void deserializeFromBytes(
        BytesShared buffer);

protected:
    /* It is necessary to use flat map here:
     * this container predicts order in which
     * this message would be transmitted between the nodes.
     *
     * In the protocol this approach is described as set of pairs <NodeUUID, vote>,
     * that is sorted in ASC order by NodeUUIDs.
     *
     * Current realisation is simplified
     * * set of pairs was replaced by the map;
     * * ascending order is provided by the flat map by default.
     */
    boost::container::flat_map<NodeUUID, Vote> mVotes;
    NodeUUID mCoordinatorUUID;

    // TODO: [mvp+] add coordinator sign to the message
    // Message may not contains coordinator vote:
    // only coordinator may create this message and in case if it is created,
    // it is assumed, that coordinator voted + for the transaction.
    //
    // But it is necessary for the intermediate nodes to be able to know,
    // if this message was really created by the coordinator,
    // to be able to protects themselves from the network spoofing.
};
#endif //GEO_NETWORK_CLIENT_PARTICIPANTSAPPROVINGMESSAGE_H
