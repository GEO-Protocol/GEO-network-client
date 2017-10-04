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
