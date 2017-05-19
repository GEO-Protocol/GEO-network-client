#include "CRC32Rt2ResponseMessage.h"

CRC32Rt2ResponseMessage::CRC32Rt2ResponseMessage(
    const NodeUUID& nodeUUID,
    uint32_t& crc32rt2sum):
    SenderMessage(nodeUUID)
{
}

const Message::MessageType CRC32Rt2ResponseMessage::typeID() const{
    return Message::RoutingTables_CRC32Rt2ResponseMessage;
}

CRC32Rt2ResponseMessage::CRC32Rt2ResponseMessage(
    BytesShared buffer) :
    SenderMessage(buffer)
{
    BytesDeserializer deserializer(
        buffer,
        SenderMessage::kOffsetToInheritedBytes());

    deserializer.copyIntoDespiteConst(&mCrc32Rt2Sum);
};

pair<BytesShared, size_t> CRC32Rt2ResponseMessage::serializeToBytes() const
throw(bad_alloc)
{
    BytesSerializer serializer;

    serializer.enqueue(SenderMessage::serializeToBytes());
    serializer.copy(mCrc32Rt2Sum);
    return serializer.collect();
};