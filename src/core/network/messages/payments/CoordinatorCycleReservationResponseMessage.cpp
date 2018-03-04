#include "CoordinatorCycleReservationResponseMessage.h"

CoordinatorCycleReservationResponseMessage::CoordinatorCycleReservationResponseMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const ResponseCycleMessage::OperationState state,
    const TrustLineAmount& reservedAmount):

    ResponseCycleMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        state),
    mAmountReserved(reservedAmount)
{}

CoordinatorCycleReservationResponseMessage::CoordinatorCycleReservationResponseMessage(
    BytesShared buffer) :

    ResponseCycleMessage(
        buffer)
{
    auto parentMessageOffset = ResponseCycleMessage::kOffsetToInheritedBytes();
    auto amountOffset = buffer.get() + parentMessageOffset;
    auto amountEndOffset = amountOffset + kTrustLineAmountBytesCount; // TODO: deserialize only non-zero
    vector<byte> amountBytes(
        amountOffset,
        amountEndOffset);

    mAmountReserved = bytesToTrustLineAmount(amountBytes);
}

const TrustLineAmount& CoordinatorCycleReservationResponseMessage::amountReserved() const
{
    return mAmountReserved;
}

pair<BytesShared, size_t> CoordinatorCycleReservationResponseMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = ResponseCycleMessage::serializeToBytes();
    auto serializedAmount = trustLineAmountToBytes(mAmountReserved);

    size_t bytesCount =
        parentBytesAndCount.second
        + serializedAmount.size();

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        serializedAmount.data(),
        serializedAmount.size());
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const Message::MessageType CoordinatorCycleReservationResponseMessage::typeID() const
{
    return Message::Payments_CoordinatorCycleReservationResponse;
}
