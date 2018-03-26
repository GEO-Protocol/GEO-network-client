/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "TransactionMessage.h"


TransactionMessage::TransactionMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID)
    noexcept :

    SenderMessage(senderUUID),
    mTransactionUUID(transactionUUID)
{}

TransactionMessage::TransactionMessage(
    BytesShared buffer)
    noexcept :

    SenderMessage(buffer),
    mTransactionUUID([&buffer](const size_t parentOffset) -> const TransactionUUID {
        TransactionUUID tu;

        memcpy(
            tu.data,
            buffer.get() + parentOffset,
            TransactionUUID::kBytesSize);

        return tu;
    }(SenderMessage::kOffsetToInheritedBytes()))
{}

const bool TransactionMessage::isTransactionMessage() const
    noexcept
{
    return true;
}

const TransactionUUID &TransactionMessage::transactionUUID() const
    noexcept
{
    return mTransactionUUID;
}

/*
 * ToDo: rewrite me with bytes deserializer
 */
pair<BytesShared, size_t> TransactionMessage::serializeToBytes() const
{
    auto parentBytesAndCount = SenderMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + TransactionUUID::kBytesSize;

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
        mTransactionUUID.data,
        TransactionUUID::kBytesSize);
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t TransactionMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const auto kOffset =
          SenderMessage::kOffsetToInheritedBytes()
        + TransactionUUID::kBytesSize;

    return kOffset;
}
