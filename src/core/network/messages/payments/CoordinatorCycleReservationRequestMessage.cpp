#include "CoordinatorCycleReservationRequestMessage.h"

CoordinatorCycleReservationRequestMessage::CoordinatorCycleReservationRequestMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount,
    const NodeUUID& nextNodeInThePath,
    uint8_t cycleLength) :

    RequestCycleMessage(
        senderUUID,
        transactionUUID,
        amount),
    mNextPathNode(nextNodeInThePath),
    mCycleLength(cycleLength)
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
    auto cycleLengthOffset = nextNodeUUIDOffset + mNextPathNode.kBytesSize;
    uint8_t *cycleLength = new (cycleLengthOffset) uint8_t;
    mCycleLength = *cycleLength;
}

const NodeUUID& CoordinatorCycleReservationRequestMessage::nextNodeInPathUUID() const
{
    return mNextPathNode;
}

uint8_t CoordinatorCycleReservationRequestMessage::cycleLength() const
{
    return mCycleLength;
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
        + mNextPathNode.kBytesSize
        + sizeof(uint8_t);

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

    auto cycleLengthOffset = nextPathNodeOffset + mNextPathNode.kBytesSize;

    memcpy(
        cycleLengthOffset,
        &mCycleLength,
        sizeof(uint8_t));

    return make_pair(
        buffer,
        totalBytesCount);
}
