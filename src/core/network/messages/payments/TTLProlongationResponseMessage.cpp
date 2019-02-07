#include "TTLProlongationResponseMessage.h"

TTLProlongationResponseMessage::TTLProlongationResponseMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> &senderAddresses,
    const TransactionUUID& transactionUUID,
    const OperationState state) :

    TransactionMessage(
        equivalent,
        senderAddresses,
        transactionUUID),
    mState(state)
{}

TTLProlongationResponseMessage::TTLProlongationResponseMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    SerializedOperationState *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
}

const TTLProlongationResponseMessage::OperationState TTLProlongationResponseMessage::state() const
{
    return mState;
}

pair<BytesShared, size_t> TTLProlongationResponseMessage::serializeToBytes() const
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount =
        parentBytesAndCount.second
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

const Message::MessageType TTLProlongationResponseMessage::typeID() const
{
    return Message::Payments_TTLProlongationResponse;
}
