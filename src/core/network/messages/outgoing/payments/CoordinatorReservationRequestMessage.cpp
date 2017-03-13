#include "CoordinatorReservationRequestMessage.h"


CoordinatorReservationRequestMessage::CoordinatorReservationRequestMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount,
    const NodeUUID& nextNodeInThePath) :

    RequestMessage(
        senderUUID,
        transactionUUID,
        amount),
    mNextPathNode(nextNodeInThePath)
{}

CoordinatorReservationRequestMessage::CoordinatorReservationRequestMessage(
    BytesShared buffer) :

    RequestMessage(buffer)
{
    deserializeFromBytes(buffer);
}


const NodeUUID& CoordinatorReservationRequestMessage::nextNodeInPathUUID() const
{
    return mNextPathNode;
}

const Message::MessageType CoordinatorReservationRequestMessage::typeID() const
{
    return Message::Payments_CoordinatorReservationRequest;
}

/**
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> CoordinatorReservationRequestMessage::serializeToBytes()
{    
    auto parentBytesAndCount = RequestMessage::serializeToBytes();
    size_t totalBytesCount =
        + parentBytesAndCount.second
        + mNextPathNode.kBytesSize;

    BytesShared buffer = tryMalloc(totalBytesCount);
    auto initialOffset = buffer.get();
    memcpy(
        initialOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);

    auto nextPathNodeOffset =
        initialOffset + parentBytesAndCount.second;

    memcpy(
        nextPathNodeOffset,
        mNextPathNode.data,
        mNextPathNode.kBytesSize);

    return make_pair(
        buffer,
        totalBytesCount);
}

void CoordinatorReservationRequestMessage::deserializeFromBytes(
    BytesShared buffer)
{
    auto parentMessageOffset = RequestMessage::kOffsetToInheritedBytes();
    auto nextNodeUUIDOffset = buffer.get() + parentMessageOffset;

    memcpy(
        mNextPathNode.data,
        nextNodeUUIDOffset,
        mNextPathNode.kBytesSize);
}


