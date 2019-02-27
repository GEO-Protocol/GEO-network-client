#ifndef GEO_NETWORK_CLIENT_NOEQUIVALENTMESSAGE_H
#define GEO_NETWORK_CLIENT_NOEQUIVALENTMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class NoEquivalentMessage : public TransactionMessage {

public:
    typedef shared_ptr<NoEquivalentMessage> Shared;

public:
    using TransactionMessage::TransactionMessage;

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_NOEQUIVALENTMESSAGE_H
