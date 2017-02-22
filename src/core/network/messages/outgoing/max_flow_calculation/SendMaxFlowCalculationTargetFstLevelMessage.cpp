//
// Created by mc on 16.02.17.
//

#include "SendMaxFlowCalculationTargetFstLevelMessage.h"

SendMaxFlowCalculationTargetFstLevelMessage::SendMaxFlowCalculationTargetFstLevelMessage(
    NodeUUID &senderUUID,
    NodeUUID &targetUUID,
    TransactionUUID &transactionUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID, transactionUUID) {};

const Message::MessageType SendMaxFlowCalculationTargetFstLevelMessage::typeID() const {

    return Message::MessageTypeID::SendMaxFlowCalculationTargetFstLevelMessageType;
}