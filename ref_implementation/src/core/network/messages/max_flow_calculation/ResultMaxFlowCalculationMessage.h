/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H

#include "../SenderMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

#include <vector>

class ResultMaxFlowCalculationMessage:
    public SenderMessage {

public:
    typedef shared_ptr<ResultMaxFlowCalculationMessage> Shared;

public:
    ResultMaxFlowCalculationMessage(
        const NodeUUID& senderUUID,
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &outgoingFlows,
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &incomingFlows);

    ResultMaxFlowCalculationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows() const;

    const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows() const;

private:
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> mOutgoingFlows;
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> mIncomingFlows;
};


#endif //GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H
