#include "ResponseMessage.h"


ResponseMessage::ResponseMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const PathUUID &pathUUID,
    const OperationState state) :

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mPathUUID(pathUUID),
    mState(state)
{}

ResponseMessage::ResponseMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    PathUUID *pathUUID = new (buffer.get() + bytesBufferOffset) PathUUID;
    mPathUUID = *pathUUID;
    bytesBufferOffset += sizeof(PathUUID);
    //----------------------------------------------------
    SerializedOperationState *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
}

const ResponseMessage::OperationState ResponseMessage::state() const
{
    return mState;
}

const Message::PathUUID ResponseMessage::pathUUID() const
{
    return mPathUUID;
}

const size_t ResponseMessage::kOffsetToInheritedBytes() const
    noexcept
{
    return TransactionMessage::kOffsetToInheritedBytes()
           + sizeof(PathUUID)
           + sizeof(SerializedOperationState);
}

/**
 *
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> ResponseMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount =
        parentBytesAndCount.second
        + sizeof(PathUUID)
        + sizeof(SerializedOperationState);

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
        &mPathUUID,
        sizeof(PathUUID));
    dataBytesOffset += sizeof(PathUUID);
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

