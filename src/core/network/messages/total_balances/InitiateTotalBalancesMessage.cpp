#include "InitiateTotalBalancesMessage.h"

InitiateTotalBalancesMessage::InitiateTotalBalancesMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID) :

    TransactionMessage(
        senderUUID,
        transactionUUID) {};

InitiateTotalBalancesMessage::InitiateTotalBalancesMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType InitiateTotalBalancesMessage::typeID() const {

    return Message::MessageTypeID::InitiateTotalBalancesMessageType;
}

