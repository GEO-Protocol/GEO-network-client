//
// Created by mc on 14.02.17.
//

#include "InitiateMaxFlowCalculationMessage.h"

InitiateMaxFlowCalculationMessage::InitiateMaxFlowCalculationMessage(
        NodeUUID &senderUUID,
        NodeUUID &targetUUID,
        TransactionUUID &transactionUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID, transactionUUID) {};

const Message::MessageType InitiateMaxFlowCalculationMessage::typeID() const {

    return Message::MessageTypeID::InitiateMaxFlowCalculationMessageType;
}
