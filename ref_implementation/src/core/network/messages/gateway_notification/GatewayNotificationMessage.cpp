/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "GatewayNotificationMessage.h"

GatewayNotificationMessage::GatewayNotificationMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const NodeState state) :

    TransactionMessage(
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