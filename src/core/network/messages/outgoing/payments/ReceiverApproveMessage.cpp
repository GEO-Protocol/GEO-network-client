#include "ReceiverApproveMessage.h"

ReceiverApproveMessage::ReceiverApproveMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const OperationState state) :

    // TODO: make TransactionMessage accept const
    TransactionMessage(
        const_cast<NodeUUID&>(senderUUID),
        const_cast<TransactionUUID&>(transactionUUID)),
    mState(state){
}

ReceiverApproveMessage::ReceiverApproveMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ReceiverApproveMessage::typeID() const {
    return Message::Payments_ReceiverApprove;
}

const ReceiverApproveMessage::OperationState ReceiverApproveMessage::state() const {
    return mState;
}

/**
 *
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> ReceiverApproveMessage::serializeToBytes() {
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

void ReceiverApproveMessage::deserializeFromBytes(BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    SerializedOperationState *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
}
