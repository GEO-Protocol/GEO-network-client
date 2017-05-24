#include "CRC32Rt2RequestMessage.h"

pair<BytesShared, size_t> CRC32Rt2RequestMessage::serializeToBytes () const
throw (bad_alloc)
{
    BytesSerializer serializer;

    serializer.enqueue(SenderMessage::serializeToBytes());
    return serializer.collect();
}

const Message::MessageType CRC32Rt2RequestMessage::typeID() const
noexcept
{
    return Message::RoutingTables_CRC32Rt2RequestMessage;
}
