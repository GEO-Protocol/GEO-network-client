/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "DebugMessage.h"


DebugMessage::DebugMessage()
    noexcept :
    TransactionMessage (
        NodeUUID::empty(),
        TransactionUUID::empty())
{}

DebugMessage::DebugMessage(
    BytesShared bytes) :
    TransactionMessage (bytes)
{
    byte someData[20000];
    memcpy(someData, bytes.get() + TransactionMessage::kOffsetToInheritedBytes(), 20000);
}

const Message::MessageType DebugMessage::typeID() const
{
    return Debug;
}

pair<BytesShared, size_t> DebugMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second + 20000;
    BytesShared dataBytesShared = tryMalloc(bytesCount);

    byte someData[20000];
    memset(someData, 0, 20000);

    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        someData,
        20000
    );
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}
