#include "RequestRoutingTablesMessage.h"

RequestRoutingTablesMessage::RequestRoutingTablesMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID) :

    TransactionMessage(
        senderUUID,
        transactionUUID) {};

RequestRoutingTablesMessage::RequestRoutingTablesMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType RequestRoutingTablesMessage::typeID() const {

    return Message::MessageTypeID::RequestRoutingTablesMessageType;
}
