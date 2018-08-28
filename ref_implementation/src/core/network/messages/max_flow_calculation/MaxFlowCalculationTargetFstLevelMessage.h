/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H

#include "../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class MaxFlowCalculationTargetFstLevelMessage :
    public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationTargetFstLevelMessage> Shared;

public:
    MaxFlowCalculationTargetFstLevelMessage(
        const NodeUUID& senderUUID,
        const NodeUUID& targetUUID);

    MaxFlowCalculationTargetFstLevelMessage(
        BytesShared buffer);

    const MessageType typeID() const;
};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H
