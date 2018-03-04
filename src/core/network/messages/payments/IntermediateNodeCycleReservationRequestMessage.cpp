#include "IntermediateNodeCycleReservationRequestMessage.h"

IntermediateNodeCycleReservationRequestMessage::IntermediateNodeCycleReservationRequestMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount,
    const NodeUUID& coordinatorUUID,
    SerializedPathLengthSize cycleLength) :

    RequestCycleMessage(
        equivalent,
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
    SerializedPathLengthSize *cycleLength = new (cycleLengthOffset) SerializedPathLengthSize;
    mCycleLength = *cycleLength;
}

const Message::MessageType IntermediateNodeCycleReservationRequestMessage::typeID() const
{
    return Message::Payments_IntermediateNodeCycleReservationRequest;
}

SerializedPathLengthSize IntermediateNodeCycleReservationRequestMessage::cycleLength() const
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
        + sizeof(SerializedPathLengthSize);

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
        sizeof(SerializedPathLengthSize));

    return make_pair(
        buffer,
        totalBytesCount);
}