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

const std::vector<NodeUUID> &NeighborsResponseMessage::neighbors () const
    noexcept
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
    throw (bad_alloc)
{
    // TODO serialization doesn't work correctly
//    BytesSerializer serializer;
//
//    serializer.enqueue(TransactionMessage::serializeToBytes());
//    serializer.enqueue((uint16_t)mNeighbors.size());
//
//    for (const auto kNeighbor : mNeighbors) {
//        cout << "NeighborsResponseMessage::serializeToBytes node: " << kNeighbor << endl;
//        serializer.enqueue(kNeighbor);
//    }
//
//    return serializer.collect();
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +
                        sizeof(uint16_t) + mNeighbors.size() * NodeUUID::kBytesSize;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    uint16_t rt1Size = (uint16_t)mNeighbors.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &rt1Size,
        sizeof(uint16_t));
    dataBytesOffset += sizeof(uint16_t);
    //----------------------------------------------------
    for (auto const &itRT1 : mNeighbors) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            itRT1.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    return make_pair(
        dataBytesShared,
        bytesCount);
}

