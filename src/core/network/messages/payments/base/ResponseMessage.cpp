#include "ResponseMessage.h"


ResponseMessage::ResponseMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const PathID &pathID,
    const OperationState state) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mPathID(pathID),
    mState(state)
{}

ResponseMessage::ResponseMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    PathID *pathID = new (buffer.get() + bytesBufferOffset) PathID;
    mPathID = *pathID;
    bytesBufferOffset += sizeof(PathID);
    //----------------------------------------------------
    SerializedOperationState *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
}

const ResponseMessage::OperationState ResponseMessage::state() const
{
    return mState;
}

const PathID ResponseMessage::pathID() const
{
    return mPathID;
}

const size_t ResponseMessage::kOffsetToInheritedBytes() const
    noexcept
{
    return TransactionMessage::kOffsetToInheritedBytes()
           + sizeof(PathID)
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
        + sizeof(PathID)
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
        &mPathID,
        sizeof(PathID));
    dataBytesOffset += sizeof(PathID);
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

