#ifndef GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONMESSAGE_H

#include "../SenderMessage.h"
#include "../result/MessageResult.h"

class InitiateMaxFlowCalculationMessage : public SenderMessage {

public:
    typedef shared_ptr<InitiateMaxFlowCalculationMessage> Shared;

public:
    InitiateMaxFlowCalculationMessage(
        const NodeUUID& senderUUID);

    InitiateMaxFlowCalculationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONMESSAGE_H
