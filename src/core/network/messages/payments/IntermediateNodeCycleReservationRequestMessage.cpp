#include "IntermediateNodeCycleReservationRequestMessage.h"

IntermediateNodeCycleReservationRequestMessage::IntermediateNodeCycleReservationRequestMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount,
    uint8_t cycleLength) :

    RequestCycleMessage(
        senderUUID,
        transactionUUID,
        amount),
    mCycleLength(cycleLength)
{}

IntermediateNodeCycleReservationRequestMessage::IntermediateNodeCycleReservationRequestMessage(
    BytesShared buffer) :

    RequestCycleMessage(buffer)
{
    auto parentMessageOffset = RequestCycleMessage::kOffsetToInheritedBytes();
    auto cycleLengthOffset = buffer.get() + parentMessageOffset;
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

/**
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> IntermediateNodeCycleReservationRequestMessage::serializeToBytes() const
throw(bad_alloc)
{
    auto parentBytesAndCount = RequestCycleMessage::serializeToBytes();
    size_t totalBytesCount =
        + parentBytesAndCount.second
        + sizeof(uint8_t);

    BytesShared buffer = tryMalloc(totalBytesCount);
    auto initialOffset = buffer.get();
    memcpy(
        initialOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);

    auto cycleLengthOffset = initialOffset + parentBytesAndCount.second;

    memcpy(
        cycleLengthOffset,
        &mCycleLength,
        sizeof(uint8_t));

    return make_pair(
        buffer,
        totalBytesCount);
}