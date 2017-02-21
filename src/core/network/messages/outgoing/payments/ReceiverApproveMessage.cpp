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

void ReceiverApproveMessage::deserializeFromBytes(BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);

    auto parentMessageOffset = TransactionMessage::kOffsetToInheritedBytes();
    auto stateOffset = buffer.get() + parentMessageOffset;
    mState = (OperationState)(SerializedOperationState(*stateOffset));
}
