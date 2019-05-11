#ifndef GEO_NETWORK_CLIENT_PONGMESSAGE_H
#define GEO_NETWORK_CLIENT_PONGMESSAGE_H

#include "../SenderMessage.h"

class PongMessage : public SenderMessage {

public:
    typedef shared_ptr<PongMessage> Shared;

public:
    PongMessage(
        ContractorID idOnReceiverSide);

    PongMessage(
        BytesShared buffer);

    const MessageType typeID() const override;
};


#endif //GEO_NETWORK_CLIENT_PONGMESSAGE_H
