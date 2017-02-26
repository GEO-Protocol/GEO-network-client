#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELINMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELINMESSAGE_H

#include "../../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class MaxFlowCalculationSourceSndLevelInMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationSourceSndLevelInMessage> Shared;

public:
    MaxFlowCalculationSourceSndLevelInMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    static const size_t kRequestedBufferSize();

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELINMESSAGE_H
