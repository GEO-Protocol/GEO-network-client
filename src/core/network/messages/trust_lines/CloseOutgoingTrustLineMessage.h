#ifndef GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H

#include "../base/transaction/DestinationMessage.h"

class CloseOutgoingTrustLineMessage : public DestinationMessage {

public:
    typedef shared_ptr<CloseOutgoingTrustLineMessage> Shared;
    typedef shared_ptr<const CloseOutgoingTrustLineMessage> ConstShared;

public:
    using DestinationMessage::DestinationMessage;

protected:
    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H
