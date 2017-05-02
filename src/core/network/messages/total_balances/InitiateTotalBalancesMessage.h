#ifndef GEO_NETWORK_CLIENT_INITIATETOTALBALANCESMESSAGE_H
#define GEO_NETWORK_CLIENT_INITIATETOTALBALANCESMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class InitiateTotalBalancesMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<InitiateTotalBalancesMessage> Shared;

public:
    using TransactionMessage::TransactionMessage;

    const MessageType typeID() const
        noexcept;
};

#endif //GEO_NETWORK_CLIENT_INITIATETOTALBALANCESMESSAGE_H
