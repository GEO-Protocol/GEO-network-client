#ifndef DEBUGMESSAGE_H
#define DEBUGMESSAGE_H

#include "../base/transaction/TransactionMessage.h"


class DebugMessage:
    public TransactionMessage {

public:
    DebugMessage();

    const MessageType typeID() const;
};

#endif // DEBUGMESSAGE_H
