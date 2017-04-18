#include "CoordinatorReservationResponseMessage.h"


CoordinatorReservationResponseMessage::CoordinatorReservationResponseMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const ResponseMessage::OperationState state,
    const TrustLineAmount& reservedAmount):

    ResponseMessage(
        senderUUID,
        transactionUUID,
        state),
    mAmountReserved(reservedAmount)
{}

CoordinatorReservationResponseMessage::CoordinatorReservationResponseMessage(
    BytesShared buffer) :

    ResponseMessage(
        buffer)
{
    deserializeFromBytes(buffer);
}

const TrustLineAmount&CoordinatorReservationResponseMessage::amountReserved() const
{
    return mAmountReserved;
}

pair<BytesShared, size_t> CoordinatorReservationResponseMessage::serializeToBytes()
{
    auto parentBytesAndCount = ResponseMessage::serializeToBytes();
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

const Message::MessageType CoordinatorReservationResponseMessage::typeID() const
{
    return Message::Payments_CoordinatorReservationResponse;
}

void CoordinatorReservationResponseMessage::deserializeFromBytes(
    BytesShared buffer)
{
    auto parentMessageOffset = ResponseMessage::kOffsetToInheritedBytes();
    auto amountOffset = buffer.get() + parentMessageOffset;
    auto amountEndOffset = amountOffset + kTrustLineAmountBytesCount; // TODO: deserialize only non-zero
    vector<byte> amountBytes(
        amountOffset,
        amountEndOffset);

    mAmountReserved = bytesToTrustLineAmount(amountBytes);
}
