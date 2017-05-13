#include "NotificationTrustLineRemovedMessage.h"

NotificationTrustLineRemovedMessage::NotificationTrustLineRemovedMessage (
    const NodeUUID &senderUUID,
    const NodeUUID &nodeA,
    const NodeUUID &nodeB,
    const byte hop)
    noexcept :

    SenderMessage(senderUUID),
    nodeA(nodeA),
    nodeB(nodeB),
    hop(hop)
{}

NotificationTrustLineRemovedMessage::NotificationTrustLineRemovedMessage (
    BytesShared buffer)
    noexcept :

    SenderMessage(buffer),
    hop(0)
{
    BytesDeserializer deserializer(
        buffer,
        SenderMessage::kOffsetToInheritedBytes());

    deserializer.copyIntoDespiteConst(&nodeA);
    deserializer.copyIntoDespiteConst(&nodeB);
    deserializer.copyIntoDespiteConst(&hop);
}

pair<BytesShared, size_t> NotificationTrustLineRemovedMessage::serializeToBytes () const
    throw (bad_alloc)
{
    BytesSerializer serializer;

    serializer.enqueue(SenderMessage::serializeToBytes());
    serializer.enqueue(nodeA);
    serializer.enqueue(nodeB);
    serializer.copy(hop);

    return serializer.collect();
}

const Message::MessageType NotificationTrustLineRemovedMessage::typeID() const
    noexcept
{
    return Message::RoutingTables_NotificationTrustLineRemoved;
}
