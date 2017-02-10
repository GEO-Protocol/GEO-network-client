#include "OperationStateMessage.h"

OperationStateMessage::OperationStateMessage(
    const OperationState state) :

    mState(state){
}

OperationStateMessage::OperationStateMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType OperationStateMessage::typeID() const {
    return Message::OperationStateMessageType;
}

const OperationStateMessage::OperationState OperationStateMessage::state() const {
    return mState;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> OperationStateMessage::serializeToBytes() const {
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount =
        + parentBytesAndCount.second
        + sizeof(SerializedOperationState);

    BytesShared buffer =
        tryMalloc(
            bytesCount);

    auto initialOffset = buffer.get();
    memcpy(
        initialOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);

    SerializedOperationState state(mState);
    auto stateOffset = initialOffset + parentBytesAndCount.second;
    memcpy(
        stateOffset,
        &state,
        sizeof(SerializedOperationState));

    return make_pair(
        buffer,
        bytesCount);
}

void OperationStateMessage::deserializeFromBytes(BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);

    auto parentMessageOffset = TransactionMessage::kOffsetToInheritedBytes();
    auto stateOffset = buffer.get() + parentMessageOffset;
    mState = (OperationState)(SerializedOperationState(*stateOffset));
}


