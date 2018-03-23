#ifndef GEO_NETWORK_CLIENT_CYCLESFOURNODESPOSITIVEBALANCEREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLESFOURNODESPOSITIVEBALANCEREQUESTMESSAGE_H

#include "CyclesFourNodesNegativeBalanceRequestMessage.h"

class CyclesFourNodesPositiveBalanceRequestMessage :
        public CyclesFourNodesNegativeBalanceRequestMessage {

public:
    typedef shared_ptr<CyclesFourNodesPositiveBalanceRequestMessage> Shared;

public:
    using CyclesFourNodesNegativeBalanceRequestMessage::CyclesFourNodesNegativeBalanceRequestMessage;

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_CYCLESFOURNODESPOSITIVEBALANCEREQUESTMESSAGE_H
