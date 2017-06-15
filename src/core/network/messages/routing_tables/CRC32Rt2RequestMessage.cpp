#include "CRC32Rt2RequestMessage.h"

pair<BytesShared, size_t> CRC32Rt2RequestMessage::serializeToBytes () const
throw (bad_alloc)
{
    BytesSerializer serializer;

    serializer.enqueue(TransactionMessage::serializeToBytes());
    uint8_t crcType = mCRC32Type;
    serializer.copy(crcType);
    return serializer.collect();
}

const Message::MessageType CRC32Rt2RequestMessage::typeID() const
noexcept
{
    return Message::RoutingTables_CRC32Rt2RequestMessage;
}

CRC32Rt2RequestMessage::CRC32Rt2RequestMessage(
    const NodeUUID &senderUIID,
    const TransactionUUID &transactionUUID,
    const CRC32SumType type):
    TransactionMessage(
        senderUIID,
        transactionUUID)
{
    mCRC32Type = type;
}

CRC32Rt2RequestMessage::CRC32Rt2RequestMessage(
    BytesShared buffer):
    TransactionMessage(buffer)
{
    auto startOffset = TransactionMessage::kOffsetToInheritedBytes();
    uint8_t crc32SumType;
    memcpy(
        &crc32SumType,
        buffer.get() + startOffset,
        sizeof(uint8_t));
    mCRC32Type = static_cast<CRC32SumType>(crc32SumType);
}

const CRC32Rt2RequestMessage::CRC32SumType CRC32Rt2RequestMessage::CRC32Type() const {
    return mCRC32Type;
}
