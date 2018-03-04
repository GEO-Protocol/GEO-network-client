#include "IntermediateNodeReservationResponseMessage.h"

IntermediateNodeReservationResponseMessage::IntermediateNodeReservationResponseMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const PathID& pathID,
    const ResponseMessage::OperationState state,
    const TrustLineAmount& reservedAmount):

    ResponseMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        pathID,
        state),
    mAmountReserved(reservedAmount)
{}

IntermediateNodeReservationResponseMessage::IntermediateNodeReservationResponseMessage(
    BytesShared buffer) :

    ResponseMessage(
        buffer)
{
    auto parentMessageOffset = ResponseMessage::kOffsetToInheritedBytes();
    auto amountOffset = buffer.get() + parentMessageOffset;
    auto amountEndOffset = amountOffset + kTrustLineAmountBytesCount; // TODO: deserialize only non-zero
    vector<byte> amountBytes(
        amountOffset,
        amountEndOffset);

    mAmountReserved = bytesToTrustLineAmount(amountBytes);
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

const Message::MessageType IntermediateNodeReservationResponseMessage::typeID() const
{
    return Message::Payments_IntermediateNodeReservationResponse;
}
