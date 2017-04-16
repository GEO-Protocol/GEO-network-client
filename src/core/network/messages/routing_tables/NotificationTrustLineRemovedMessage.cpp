#include "NotificationTrustLineRemovedMessage.h"

NotificationTrustLineRemovedMessage::NotificationTrustLineRemovedMessage (
    const NodeUUID &senderUUID,
    const NodeUUID &nodeA,
    const NodeUUID &nodeB,
    const byte hop):

    SenderMessage(senderUUID),
    nodeA(nodeA),
    nodeB(nodeB),
    hop(hop)
{}

NotificationTrustLineRemovedMessage::NotificationTrustLineRemovedMessage (
    BytesShared buffer):

    SenderMessage(buffer),
    hop(0)
{
    BytesDeserializer memory(
        buffer,
        SenderMessage::kOffsetToInheritedBytes());

    memory.copyInto(nodeA);
    memory.copyInto(nodeB);
    memory.copyInto(hop);
}

pair<BytesShared, size_t> NotificationTrustLineRemovedMessage::serializeToBytes () const
{
    BytesSerializer serializer;

    serializer.enqueue(SenderMessage::serializeToBytes());
    serializer.enqueue(nodeA);
    serializer.enqueue(nodeB);
    serializer.enqueue(hop);

    return serializer.collect();
}

const MessageType NotificationTrustLineRemovedMessage::typeID() const
    noexcept
{
    return Message::RoutingTables_NotificationTrustLineRemoved;
}