#ifndef GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class CloseOutgoingTrustLineMessage : public TransactionMessage {

public:
    typedef shared_ptr<CloseOutgoingTrustLineMessage> Shared;
    typedef shared_ptr<const CloseOutgoingTrustLineMessage> ConstShared;

public:
    using TransactionMessage::TransactionMessage;

protected:
    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H
