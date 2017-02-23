//
// Created by mc on 16.02.17.
//

#include "SendMaxFlowCalculationSourceFstLevelMessage.h"

SendMaxFlowCalculationSourceFstLevelMessage::SendMaxFlowCalculationSourceFstLevelMessage(
    NodeUUID &senderUUID,
    NodeUUID &targetUUID,
    TransactionUUID &transactionUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID, transactionUUID) {};

const Message::MessageType SendMaxFlowCalculationSourceFstLevelMessage::typeID() const {

    return Message::MessageTypeID::SendMaxFlowCalculationSourceFstLevelMessageType;
}
