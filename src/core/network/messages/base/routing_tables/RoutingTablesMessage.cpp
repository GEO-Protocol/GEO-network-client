#include "RoutingTablesMessage.h"

RoutingTablesMessage::RoutingTablesMessage() {}

RoutingTablesMessage::RoutingTablesMessage(
    const NodeUUID &senderUUID) :

    SenderMessage(senderUUID) {}

const bool RoutingTablesMessage::isRoutingTableMessage() const {

    return true;
}
