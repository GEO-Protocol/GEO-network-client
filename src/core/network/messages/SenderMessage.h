#ifndef GEO_NETWORK_CLIENT_SENDERMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDERMESSAGE_H

#include "Message.hpp"

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../common/memory/MemoryUtils.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class SenderMessage : public Message {

public:
    const NodeUUID &senderUUID() const;

protected:
    SenderMessage();

    SenderMessage(
        const NodeUUID &senderUUID);

    virtual const MessageType typeID() const = 0;

    virtual pair<BytesShared, size_t> serializeToBytes();

    virtual void deserializeFromBytes(
        BytesShared buffer);

    static const size_t kOffsetToInheritedBytes();

protected:
    NodeUUID mSenderUUID;
};
#endif //GEO_NETWORK_CLIENT_SENDERMESSAGE_H
