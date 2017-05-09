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

    const MessageType typeID() const;
};

#endif // DEBUGMESSAGE_H
