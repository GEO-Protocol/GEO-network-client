#include "CoordinatorCycleReservationRequestMessage.h"

CoordinatorCycleReservationRequestMessage::CoordinatorCycleReservationRequestMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount,
    const NodeUUID& nextNodeInThePath) :

    RequestCycleMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        amount),
    mNextPathNode(nextNodeInThePath)
{}

CoordinatorCycleReservationRequestMessage::CoordinatorCycleReservationRequestMessage(
    BytesShared buffer) :

    RequestCycleMessage(buffer)
{
    auto parentMessageOffset = RequestCycleMessage::kOffsetToInheritedBytes();
    auto nextNodeUUIDOffset = buffer.get() + parentMessageOffset;

    memcpy(
        mNextPathNode.data,
        nextNodeUUIDOffset,
        mNextPathNode.kBytesSize);
}

const NodeUUID& CoordinatorCycleReservationRequestMessage::nextNodeInPath() const
{
    return mNextPathNode;
}

const Message::MessageType CoordinatorCycleReservationRequestMessage::typeID() const
{
    return Message::Payments_CoordinatorCycleReservationRequest;
}

/**
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> CoordinatorCycleReservationRequestMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = RequestCycleMessage::serializeToBytes();
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
