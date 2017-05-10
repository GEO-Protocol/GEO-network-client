#ifndef GEO_NETWORK_CLIENT_TTLPROLONGATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_TTLPROLONGATIONMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class TTLPolongationMessage : public TransactionMessage {

public:
    typedef shared_ptr<TTLPolongationMessage> Shared;
    typedef shared_ptr<const TTLPolongationMessage> ConstShared;

public:
    using TransactionMessage::TransactionMessage;

protected:
    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_TTLPROLONGATIONMESSAGE_H
