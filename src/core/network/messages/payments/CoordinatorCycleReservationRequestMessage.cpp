#include "CoordinatorCycleReservationRequestMessage.h"

CoordinatorCycleReservationRequestMessage::CoordinatorCycleReservationRequestMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> senderAddresses,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount,
    BaseAddress::Shared nextNodeInThePathAddress) :

    RequestCycleMessage(
        equivalent,
        senderAddresses,
        transactionUUID,
        amount),
    mNextPathNodeAddress(nextNodeInThePathAddress)
{}

CoordinatorCycleReservationRequestMessage::CoordinatorCycleReservationRequestMessage(
    BytesShared buffer) :

    RequestCycleMessage(buffer)
{
    auto bytesBufferOffset = buffer.get() + RequestCycleMessage::kOffsetToInheritedBytes();

    mNextPathNodeAddress = deserializeAddress(bytesBufferOffset);
}

BaseAddress::Shared CoordinatorCycleReservationRequestMessage::nextNodeInPathAddress() const
{
    return mNextPathNodeAddress;
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
        + mNextPathNodeAddress->serializedSize();

    BytesShared buffer = tryMalloc(totalBytesCount);
    auto bytesBufferOffset = buffer.get();
    memcpy(
        bytesBufferOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    bytesBufferOffset += parentBytesAndCount.second;

    auto serializedAddress = mNextPathNodeAddress->serializeToBytes();
    memcpy(
        bytesBufferOffset,
        serializedAddress.get(),
        mNextPathNodeAddress->serializedSize());

    return make_pair(
        buffer,
        totalBytesCount);
}
