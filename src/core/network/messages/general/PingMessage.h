#ifndef GEO_NETWORK_CLIENT_PINGMESSAGE_H
#define GEO_NETWORK_CLIENT_PINGMESSAGE_H

#include "../SenderMessage.h"

class PingMessage : public SenderMessage {

public:
    typedef shared_ptr<PingMessage> Shared;

public:
    PingMessage(
        ContractorID idOnReceiverSide);

    PingMessage(
        BytesShared buffer);

    const MessageType typeID() const override;
};


#endif //GEO_NETWORK_CLIENT_PINGMESSAGE_H
