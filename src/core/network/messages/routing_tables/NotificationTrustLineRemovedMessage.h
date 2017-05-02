#ifndef GEO_NETWORK_CLIENT_TRUSTLINEREMOVEDNOTIFICATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINEREMOVEDNOTIFICATIONMESSAGE_H

#include "../SenderMessage.h"


/*
 * This message is used to notify neighbor nodes about trust line removing.
 * It contains pair of nodes UUIDs (A and B), but no direction info about removed trust line.
 * So, the receiver node must try to remove both (A -> B) and (B -> A) trust lines.
 * It is safe to delete both trust lines: in case if this message was emitted -
 * there is no ANY trust line between nodes. (Trust line updated notification message is used in other cases).
 */
class NotificationTrustLineRemovedMessage:
    public SenderMessage {

public:
    typedef shared_ptr<NotificationTrustLineRemovedMessage> Shared;

public:
    NotificationTrustLineRemovedMessage(
        const NodeUUID &senderUUID,
        const NodeUUID &nodeA,
        const NodeUUID &nodeB,
        const byte hop=0)
        noexcept;

    NotificationTrustLineRemovedMessage(
        BytesShared buffer)
        noexcept;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

    virtual const MessageType typeID() const
        noexcept;

public:
    const NodeUUID nodeA;
    const NodeUUID nodeB;

    /*
     * This notification message may be propagated further
     * (from the receiver node to it's first level nodes),
     * for them to be able to update their routing tables.
     *
     * This parameter contains current hop-level of the message.
     */
    const byte hop;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINEREMOVEDNOTIFICATIONMESSAGE_H
