#include "IntermediateNodeReservationResponse.h"


IntermediateNodeReservationResponse::IntermediateNodeReservationResponse(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const OperationState state) :

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mState(state)
{}

IntermediateNodeReservationResponse::IntermediateNodeReservationResponse(
    BytesShared buffer)
{
    deserializeFromBytes(buffer);
}

const Message::MessageType IntermediateNodeReservationResponse::typeID() const
{
    return Message::Payments_IntermediateNodeReservationResponse;
}

const IntermediateNodeReservationResponse::OperationState IntermediateNodeReservationResponse::state() const
{
    return mState;
}

/**
 *
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> IntermediateNodeReservationResponse::serializeToBytes()
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount =
            parentBytesAndCount.second + sizeof(SerializedOperationState);

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    SerializedOperationState state(mState);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &state,
        sizeof(SerializedOperationState));
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void IntermediateNodeReservationResponse::deserializeFromBytes(BytesShared buffer)
{
    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    SerializedOperationState *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
}
