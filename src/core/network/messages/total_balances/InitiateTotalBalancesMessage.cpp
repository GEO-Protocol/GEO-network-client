#include "InitiateTotalBalancesMessage.h"

InitiateTotalBalancesMessage::InitiateTotalBalancesMessage(
        const NodeUUID& senderUUID) :

        SenderMessage(senderUUID) {};

InitiateTotalBalancesMessage::InitiateTotalBalancesMessage(
        BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType InitiateTotalBalancesMessage::typeID() const {

    return Message::MessageTypeID::InitiateTotalBalancesMessageType;
}

const bool InitiateTotalBalancesMessage::isTotalBalancesResponseMessage() const {
    return true;
}

