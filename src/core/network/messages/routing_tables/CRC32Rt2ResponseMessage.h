#ifndef GEO_NETWORK_CLIENT_CRC32RT2RESPONSETMESSAGE_H
#define GEO_NETWORK_CLIENT_CRC32RT2RESPONSETMESSAGE_H

#include "../SenderMessage.h"


class CRC32Rt2ResponseMessage :
    public  SenderMessage{

public:

    CRC32Rt2ResponseMessage(
        const NodeUUID& nodeUUID,
        uint32_t& crc32rt2sum);

    CRC32Rt2ResponseMessage(
        BytesShared buffer);

    pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

public:
    const MessageType typeID() const;

protected:
    uint32_t mCrc32Rt2Sum;

};

#endif //GEO_NETWORK_CLIENT_CRC32RT2RESPONSETMESSAGE_H
