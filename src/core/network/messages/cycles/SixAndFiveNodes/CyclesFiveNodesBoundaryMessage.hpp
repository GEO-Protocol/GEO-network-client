#ifndef GEO_NETWORK_CLIENT_CYCLEFIVENODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEFIVENODESBOUNDARYMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesBoundaryMessage.h"

class CyclesFiveNodesBoundaryMessage:
    public CyclesBaseFiveOrSixNodesBoundaryMessage {

public:
    using CyclesBaseFiveOrSixNodesBoundaryMessage::CyclesBaseFiveOrSixNodesBoundaryMessage;

    const MessageType typeID() const override {
        return Message::Cycles_FiveNodesBoundary;
    };
};
#endif //GEO_NETWORK_CLIENT_CYCLEFIVENODESBOUNDARYMESSAGE_H
