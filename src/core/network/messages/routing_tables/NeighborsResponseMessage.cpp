#include "NeighborsResponseMessage.h"


/**
 * @param expectedNodesCount is used for optimized nodes appending
 * (memory reservation would be done for this amount of nodes).
 */
NeighborsResponseMessage::NeighborsResponseMessage (
    const NodeUUID &senderUIID,
    const TransactionUUID &transactionUUID,
    const uint16_t expectedNodesCount)
    noexcept :

    TransactionMessage(
        senderUIID,
        transactionUUID)
{
    if (expectedNodesCount > 0)
        mNeighbors.reserve(expectedNodesCount);
}

NeighborsResponseMessage::NeighborsResponseMessage (
    BytesShared buffer)
    noexcept :

    TransactionMessage(buffer)
{
    BytesDeserializer memory(
        buffer,
        TransactionMessage::kOffsetToInheritedBytes());

    uint16_t recordsCount = 0;
    memory.copyInto(&recordsCount);
    mNeighbors.reserve(recordsCount);

    NodeUUID uuid;
    for (uint16_t i=0; i<recordsCount; ++i){
        memory.copyInto(&uuid);
        mNeighbors.push_back(uuid);
    }
}

const Message::MessageType NeighborsResponseMessage::typeID () const
    noexcept
{
    return Message::RoutingTables_NeighborsResponse;
}

std::vector &NeighborsResponseMessage::neighbors () const
{
    return mNeighbors;
}

void NeighborsResponseMessage::appendNeighbor (
    const NodeUUID &nodeUUID)
    throw (bad_alloc, OverflowError)
{
    if (mNeighbors.size() >= kMaxNeighborNodesCount)
        throw OverflowError(
            "NeighborsResponseMessage::neighbors (): "
                "no more neighbors can be inserted into the message.");

    mNeighbors.push_back(nodeUUID);
}

pair<BytesShared, size_t> NeighborsResponseMessage::serializeToBytes () const
    noexcept
{
    BytesSerializer serializer;

    serializer.enqueue(TransactionMessage::serializeToBytes());
    serializer.enqueue((uint16_t)mNeighbors.size());

    for (const auto kNeighbor : mNeighbors)
        serializer.enqueue(kNeighbor);

    return serializer.collect();
}

