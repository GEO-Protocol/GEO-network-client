//
// Created by mc on 17.02.17.
//

#include "MaxFlowCalculationTargetFstLevelOutMessage.h"

MaxFlowCalculationTargetFstLevelOutMessage::MaxFlowCalculationTargetFstLevelOutMessage(
    NodeUUID &senderUUID,
    NodeUUID &targetUUID,
    TransactionUUID &transactionUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID, transactionUUID) {};

const Message::MessageType MaxFlowCalculationTargetFstLevelOutMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationTargetFstLevelOutMessageType;
}
