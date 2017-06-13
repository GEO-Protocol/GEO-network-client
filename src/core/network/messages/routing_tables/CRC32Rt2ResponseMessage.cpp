#include "CRC32Rt2ResponseMessage.h"

CRC32Rt2ResponseMessage::CRC32Rt2ResponseMessage(
    const NodeUUID& nodeUUID,
    const TransactionUUID &transactionUUID,
    uint32_t& crc32rt2sum):
    TransactionMessage(
        nodeUUID,
        transactionUUID)
{
    mCrc32Rt2Sum  = crc32rt2sum;
}

const Message::MessageType CRC32Rt2ResponseMessage::typeID() const
noexcept
{
    return Message::RoutingTables_CRC32Rt2ResponseMessage;
}

CRC32Rt2ResponseMessage::CRC32Rt2ResponseMessage(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    BytesDeserializer deserializer(
        buffer,
        TransactionMessage::kOffsetToInheritedBytes());
    deserializer.copyIntoDespiteConst(&mCrc32Rt2Sum);
};

pair<BytesShared, size_t> CRC32Rt2ResponseMessage::serializeToBytes() const
throw(bad_alloc)
{
    BytesSerializer serializer;

    serializer.enqueue(TransactionMessage::serializeToBytes());
    serializer.copy(mCrc32Rt2Sum);
    return serializer.collect();
};

uint32_t CRC32Rt2ResponseMessage::crc32Rt2Sum() const
{
    return mCrc32Rt2Sum;
}