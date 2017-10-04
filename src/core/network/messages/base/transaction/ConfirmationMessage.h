#ifndef CONFIRMATIONMESSAGE_H
#define CONFIRMATIONMESSAGE_H

#include "TransactionMessage.h"


class ConfirmationMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<ConfirmationMessage> Shared;

public:
    using TransactionMessage::TransactionMessage;

    const MessageType typeID() const
        noexcept;
};

#endif // CONFIRMATIONMESSAGE_H
