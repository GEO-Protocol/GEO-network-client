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
