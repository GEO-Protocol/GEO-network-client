//
// Created by mc on 16.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELINMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELINMESSAGE_H

#include "../../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class MaxFlowCalculationTargetFstLevelInMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationTargetFstLevelInMessage> Shared;

public:
    MaxFlowCalculationTargetFstLevelInMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    static const size_t kRequestedBufferSize();

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELINMESSAGE_H
