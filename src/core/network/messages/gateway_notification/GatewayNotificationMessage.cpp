#include "GatewayNotificationMessage.h"

GatewayNotificationMessage::GatewayNotificationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const NodeState state) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mNodeState(state)
{}

GatewayNotificationMessage::GatewayNotificationMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    SerializedNodeState *state = new (buffer.get() + bytesBufferOffset) SerializedNodeState;
    mNodeState = (NodeState) (*state);
}

const GatewayNotificationMessage::NodeState GatewayNotificationMessage::nodeState() const
{
    return mNodeState;
}

pair<BytesShared, size_t> GatewayNotificationMessage::serializeToBytes() const
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount =
        parentBytesAndCount.second
        + sizeof(SerializedNodeState);

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    SerializedNodeState state(mNodeState);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &state,
        sizeof(SerializedNodeState));
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const Message::MessageType GatewayNotificationMessage::typeID() const
{
    return Message::GatewayNotification;
}