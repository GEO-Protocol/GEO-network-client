#include "ResultMaxFlowCalculationGatewayMessage.h"

const Message::MessageType ResultMaxFlowCalculationGatewayMessage::typeID() const
{
    return Message::MaxFlow_ResultMaxFlowCalculationFromGateway;
}