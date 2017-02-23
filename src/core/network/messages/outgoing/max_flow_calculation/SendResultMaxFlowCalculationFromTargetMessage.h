//
// Created by mc on 15.02.17.
//

#ifndef GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONFROMTARGETMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONFROMTARGETMESSAGE_H

#include "../../base/max_flow_calculation/MaxFlowCalculationMessage.h"
#include "../../result/MessageResult.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class SendResultMaxFlowCalculationFromTargetMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<SendResultMaxFlowCalculationFromTargetMessage> Shared;

public:

    SendResultMaxFlowCalculationFromTargetMessage(
        NodeUUID &senderUUID,
        NodeUUID &targetUUID,
        TransactionUUID &transactionUUID,
        map<NodeUUID, TrustLineAmount> &incomingFlows);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

private:

    void deserializeFromBytes(
        BytesShared buffer);

private:

    map<NodeUUID, TrustLineAmount> mIncomingFlows;

};


#endif //GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONFROMTARGETMESSAGE_H
