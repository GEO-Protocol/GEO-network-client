#include "CRC32ThirdLevelResponseMessage.h"

CRC32ThirdLevelResponseMessage::CRC32ThirdLevelResponseMessage(
    const NodeUUID &senderUIID,
    const TransactionUUID &transactionUUID,
    vector<pair<NodeUUID, uint32_t >> crc32sums):
    TransactionMessage(
        senderUIID,
        transactionUUID)
{
    mCRC32Sums = crc32sums;
}

CRC32ThirdLevelResponseMessage::CRC32ThirdLevelResponseMessage(
    BytesShared buffer):
    TransactionMessage(buffer)
{

}

pair<BytesShared, size_t> CRC32ThirdLevelResponseMessage::serializeToBytes() const
throw(bad_alloc)
{
    return TransactionMessage::serializeToBytes();
}

const Message::MessageType CRC32ThirdLevelResponseMessage::typeID() const
noexcept
{
    return Message::RoutingTables_CRC32Rt2ThirdLevelResponseMessage;
}

const vector<pair<NodeUUID, uint32_t>> CRC32ThirdLevelResponseMessage::crc32Sums() const {
    return mCRC32Sums;
}
