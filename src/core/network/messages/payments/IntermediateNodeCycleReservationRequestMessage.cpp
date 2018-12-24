#include "IntermediateNodeCycleReservationRequestMessage.h"

IntermediateNodeCycleReservationRequestMessage::IntermediateNodeCycleReservationRequestMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> senderAddresses,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount,
    BaseAddress::Shared coordinatorAddress,
    SerializedPathLengthSize cycleLength) :

    RequestCycleMessage(
        equivalent,
        senderAddresses,
        transactionUUID,
        amount),
    mCoordinatorAddress(coordinatorAddress),
    mCycleLength(cycleLength)
{}

IntermediateNodeCycleReservationRequestMessage::IntermediateNodeCycleReservationRequestMessage(
    BytesShared buffer) :

    RequestCycleMessage(buffer)
{
    auto parentMessageOffset = RequestCycleMessage::kOffsetToInheritedBytes();
    auto dataBytesOffset = buffer.get() + parentMessageOffset;

    mCoordinatorAddress = deserializeAddress(dataBytesOffset);
    dataBytesOffset += mCoordinatorAddress->serializedSize();

    SerializedPathLengthSize *cycleLength = new (dataBytesOffset) SerializedPathLengthSize;
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

BaseAddress::Shared IntermediateNodeCycleReservationRequestMessage::coordinatorAddress() const
{
    return mCoordinatorAddress;
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
        + mCoordinatorAddress->serializedSize()
        + sizeof(SerializedPathLengthSize);

    BytesShared buffer = tryMalloc(totalBytesCount);
    auto bytesBufferOffset = buffer.get();
    memcpy(
        bytesBufferOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    bytesBufferOffset += parentBytesAndCount.second;

    auto serializedAddress = mCoordinatorAddress->serializeToBytes();
    memcpy(
        bytesBufferOffset,
        serializedAddress.get(),
        mCoordinatorAddress->serializedSize());
    bytesBufferOffset += mCoordinatorAddress->serializedSize();

    memcpy(
        bytesBufferOffset,
        &mCycleLength,
        sizeof(SerializedPathLengthSize));

    return make_pair(
        buffer,
        totalBytesCount);
}