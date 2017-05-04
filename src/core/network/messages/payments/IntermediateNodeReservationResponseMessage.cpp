#include "IntermediateNodeReservationResponseMessage.h"

IntermediateNodeReservationResponseMessage::IntermediateNodeReservationResponseMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const PathUUID& pathUUID,
    const ResponseMessage::OperationState state,
    const TrustLineAmount& reservedAmount):

    ResponseMessage(
        senderUUID,
        transactionUUID,
        pathUUID,
        state),
    mAmountReserved(reservedAmount)
{}

IntermediateNodeReservationResponseMessage::IntermediateNodeReservationResponseMessage(
    BytesShared buffer) :

    ResponseMessage(
        buffer)
{
    deserializeFromBytes(buffer);
}

const TrustLineAmount& IntermediateNodeReservationResponseMessage::amountReserved() const
{
    return mAmountReserved;
}

pair<BytesShared, size_t> IntermediateNodeReservationResponseMessage::serializeToBytes() const
throw(bad_alloc)
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

void IntermediateNodeReservationResponseMessage::deserializeFromBytes(
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

const Message::MessageType IntermediateNodeReservationResponseMessage::typeID() const
{
    return Message::Payments_IntermediateNodeReservationResponse;
}
