#ifndef GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONGATEWAYMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONGATEWAYMESSAGE_H

#include "ResultMaxFlowCalculationMessage.h"

class ResultMaxFlowCalculationGatewayMessage :
    public ResultMaxFlowCalculationMessage {

public:
    typedef shared_ptr<ResultMaxFlowCalculationGatewayMessage> Shared;

public:
    using ResultMaxFlowCalculationMessage::ResultMaxFlowCalculationMessage;

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONGATEWAYMESSAGE_H
