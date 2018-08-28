/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef DEBUGMESSAGE_H
#define DEBUGMESSAGE_H

#include "../base/transaction/TransactionMessage.h"


class DebugMessage:
    public TransactionMessage {

public:
    DebugMessage()
        noexcept;

    DebugMessage(
        BytesShared bytes);

    virtual const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);
};

#endif // DEBUGMESSAGE_H
