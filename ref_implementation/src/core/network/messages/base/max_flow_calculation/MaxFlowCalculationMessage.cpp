/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "MaxFlowCalculationMessage.h"


MaxFlowCalculationMessage::MaxFlowCalculationMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    SenderMessage(senderUUID),

    mTargetUUID(targetUUID)
{}

MaxFlowCalculationMessage::MaxFlowCalculationMessage (
    BytesShared buffer) :
    SenderMessage(buffer)
{
    memcpy(
        mTargetUUID.data,
        buffer.get() + SenderMessage::kOffsetToInheritedBytes(),
        NodeUUID::kBytesSize);
}

const NodeUUID &MaxFlowCalculationMessage::targetUUID() const
{
    return mTargetUUID;
}

pair<BytesShared, size_t> MaxFlowCalculationMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = SenderMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second + NodeUUID::kBytesSize;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
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
        mTargetUUID.data,
        NodeUUID::kBytesSize);
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void MaxFlowCalculationMessage::deserializeFromBytes(
    BytesShared buffer)
{
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        mTargetUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
}

const size_t MaxFlowCalculationMessage::kOffsetToInheritedBytes()
{
    static const size_t offset = sizeof(MessageType) + NodeUUID::kBytesSize;
    return offset;
}
