#include "InitiateMaxFlowCalculationMessage.h"


const Message::MessageType InitiateMaxFlowCalculationMessage::typeID() const
{
    return Message::MaxFlow_InitiateCalculation;
}
