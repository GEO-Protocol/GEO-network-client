//
// Created by mc on 16.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELINMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELINMESSAGE_H

#include "../../MaxFlowCalculationMessage.hpp"

class MaxFlowCalculationSourceFstLevelInMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationSourceFstLevelInMessage> Shared;

public:
    MaxFlowCalculationSourceFstLevelInMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    static const size_t kRequestedBufferSize();
};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELINMESSAGE_H
