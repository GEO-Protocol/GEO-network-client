#ifndef GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONMESSAGE_H

#include "../../result/MessageResult.h"
#include "../../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class ReceiveMaxFlowCalculationOnTargetMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<ReceiveMaxFlowCalculationOnTargetMessage> Shared;

public:
    ReceiveMaxFlowCalculationOnTargetMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    static const size_t kRequestedBufferSize();

};


#endif //GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONMESSAGE_H
