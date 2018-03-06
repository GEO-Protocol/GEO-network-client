#include "EquivalentMessage.h"

EquivalentMessage::EquivalentMessage(
    const SerializedEquivalent equivalent)
    noexcept:
    mEquivalent(equivalent)
{}

EquivalentMessage::EquivalentMessage(
    BytesShared buffer)
    noexcept
{
    memcpy(
        &mEquivalent,
        buffer.get() + Message::kOffsetToInheritedBytes(),
        sizeof(SerializedEquivalent));
}

const SerializedEquivalent EquivalentMessage::equivalent() const
{
    return mEquivalent;
}

pair<BytesShared, size_t> EquivalentMessage::serializeToBytes() const
{
    auto parentBytesAndCount = Message::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedEquivalent);

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
        &mEquivalent,
        sizeof(SerializedEquivalent));
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t EquivalentMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const auto kOffset =
            Message::kOffsetToInheritedBytes()
            + sizeof(SerializedEquivalent);
    return kOffset;
}
