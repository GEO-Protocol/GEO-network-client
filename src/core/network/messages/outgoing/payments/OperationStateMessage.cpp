#include "OperationStateMessage.h"

OperationStateMessage::OperationStateMessage(
    const OperationState state) :

    mState(state) {}

OperationStateMessage::OperationStateMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType OperationStateMessage::typeID() const {

    return Message::OperationStateMessageType;
}

const TransactionUUID &OperationStateMessage::transactionUUID() const {

    throw NotImplementedError("OperationStateMessage: public TransactionMessage::transactionUUID: "
                                  "Method not implemented.");
}

const OperationStateMessage::OperationState OperationStateMessage::state() const {

    return mState;
}

pair<BytesShared, size_t> OperationStateMessage::serializeToBytes() {

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedOperationState);

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    SerializedOperationState state(mState);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &state,
        sizeof(SerializedOperationState)
    );
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void OperationStateMessage::deserializeFromBytes(
    BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    SerializedOperationState *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
}