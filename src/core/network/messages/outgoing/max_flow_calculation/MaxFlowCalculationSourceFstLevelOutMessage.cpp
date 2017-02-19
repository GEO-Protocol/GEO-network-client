//
// Created by mc on 17.02.17.
//

#include "MaxFlowCalculationSourceFstLevelOutMessage.h"

MaxFlowCalculationSourceFstLevelOutMessage::MaxFlowCalculationSourceFstLevelOutMessage(
    NodeUUID &senderUUID,
    NodeUUID &targetUUID,
    TransactionUUID &transactionUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID, transactionUUID) {};

const Message::MessageType MaxFlowCalculationSourceFstLevelOutMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceFstLevelOutMessageType;
}
