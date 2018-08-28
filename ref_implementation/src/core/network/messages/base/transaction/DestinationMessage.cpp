/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "DestinationMessage.h"

DestinationMessage::DestinationMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const NodeUUID &destinationUUID)
    noexcept :

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mDestinationUUID(destinationUUID)
{}

DestinationMessage::DestinationMessage(
    BytesShared buffer)
    noexcept :

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    memcpy(
        mDestinationUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
}

const NodeUUID &DestinationMessage::destinationUUID() const
noexcept
{
    return mDestinationUUID;
}

/*
 * ToDo: rewrite me with bytes deserializer
 */
pair<BytesShared, size_t> DestinationMessage::serializeToBytes() const
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                    + NodeUUID::kBytesSize;

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
        mDestinationUUID.data,
        NodeUUID::kBytesSize);
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t DestinationMessage::kOffsetToInheritedBytes() const
noexcept
{
    static const auto kOffset =
        TransactionMessage::kOffsetToInheritedBytes()
        + NodeUUID::kBytesSize;

    return kOffset;
}
