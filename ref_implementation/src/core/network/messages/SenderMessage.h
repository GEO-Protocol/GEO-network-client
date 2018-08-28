/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_SENDERMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDERMESSAGE_H


#include "Message.hpp"

#include "../../common/NodeUUID.h"
#include "../../common/serialization/BytesDeserializer.h"
#include "../../common/serialization/BytesSerializer.h"


/*
 * Abstract base class for messages that must contain sender node UUID.
 */
class SenderMessage:
    public Message {

public:
    const NodeUUID senderUUID;

public:
    SenderMessage(
        const NodeUUID &senderUUID)
        noexcept;

    SenderMessage(
        BytesShared buffer)
        noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    virtual const size_t kOffsetToInheritedBytes() const
        noexcept;
};

#endif //GEO_NETWORK_CLIENT_SENDERMESSAGE_H
