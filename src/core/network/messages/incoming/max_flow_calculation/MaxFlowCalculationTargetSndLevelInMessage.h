//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELINMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELINMESSAGE_H

#include "../../MaxFlowCalculationMessage.hpp"

class MaxFlowCalculationTargetSndLevelInMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationTargetSndLevelInMessage> Shared;

public:
    MaxFlowCalculationTargetSndLevelInMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    static const size_t kRequestedBufferSize();

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELINMESSAGE_H
