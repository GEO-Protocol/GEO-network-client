#include "IntermediateNodeCycleReservationRequestMessage.h"

IntermediateNodeCycleReservationRequestMessage::IntermediateNodeCycleReservationRequestMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount,
    const NodeUUID& coordinatorUUID,
    uint8_t cycleLength) :

    RequestCycleMessage(
        senderUUID,
        transactionUUID,
        amount),
    mCoordinatorUUID(coordinatorUUID),
    mCycleLength(cycleLength)
{}

IntermediateNodeCycleReservationRequestMessage::IntermediateNodeCycleReservationRequestMessage(
    BytesShared buffer) :

    RequestCycleMessage(buffer)
{
    auto parentMessageOffset = RequestCycleMessage::kOffsetToInheritedBytes();
    auto coordinatorUUIDOffset = buffer.get() + parentMessageOffset;
    memcpy(
        mCoordinatorUUID.data,
        coordinatorUUIDOffset,
        NodeUUID::kBytesSize);
    auto cycleLengthOffset = coordinatorUUIDOffset + NodeUUID::kBytesSize;
    uint8_t *cycleLength = new (cycleLengthOffset) uint8_t;
    mCycleLength = *cycleLength;
}

const Message::MessageType IntermediateNodeCycleReservationRequestMessage::typeID() const
{
    return Message::Payments_IntermediateNodeCycleReservationRequest;
}

uint8_t IntermediateNodeCycleReservationRequestMessage::cycleLength() const
{
    return mCycleLength;
}

const NodeUUID& IntermediateNodeCycleReservationRequestMessage::coordinatorUUID() const
{
    return mCoordinatorUUID;
}

/**
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> IntermediateNodeCycleReservationRequestMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = RequestCycleMessage::serializeToBytes();
    size_t totalBytesCount =
        + parentBytesAndCount.second
        + NodeUUID::kBytesSize
        + sizeof(uint8_t);

    BytesShared buffer = tryMalloc(totalBytesCount);
    auto initialOffset = buffer.get();
    memcpy(
        initialOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);

    auto coordinatorUUIDOffset = initialOffset + parentBytesAndCount.second;
    memcpy(
        coordinatorUUIDOffset,
        mCoordinatorUUID.data,
        NodeUUID::kBytesSize);

    auto cycleLengthOffset = coordinatorUUIDOffset + NodeUUID::kBytesSize;
    memcpy(
        cycleLengthOffset,
        &mCycleLength,
        sizeof(uint8_t));

    return make_pair(
        buffer,
        totalBytesCount);
}