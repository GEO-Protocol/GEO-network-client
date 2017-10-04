#ifndef GEO_NETWORK_CLIENT_TTLPROLONGATIONREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_TTLPROLONGATIONREQUESTMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class TTLProlongationRequestMessage : public TransactionMessage {

public:
    typedef shared_ptr<TTLProlongationRequestMessage> Shared;
    typedef shared_ptr<const TTLProlongationRequestMessage> ConstShared;

public:
    using TransactionMessage::TransactionMessage;

protected:
    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_TTLPROLONGATIONREQUESTMESSAGE_H
