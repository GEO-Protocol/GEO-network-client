#include "RoutingTablesMessage.h"

RoutingTablesMessage::RoutingTablesMessage() {}

RoutingTablesMessage::RoutingTablesMessage(
    const NodeUUID &senderUUID) :

    SenderMessage(senderUUID) {}
