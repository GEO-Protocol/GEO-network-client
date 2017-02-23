//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONFROMSOURCEMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONFROMSOURCEMESSAGE_H

#include "../../base/max_flow_calculation/MaxFlowCalculationMessage.h"
#include "../../result/MessageResult.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class SendResultMaxFlowCalculationFromSourceMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<SendResultMaxFlowCalculationFromSourceMessage> Shared;

public:

    SendResultMaxFlowCalculationFromSourceMessage(
        NodeUUID &senderUUID,
        NodeUUID &targetUUID,
        TransactionUUID &transactionUUID,
        map<NodeUUID, TrustLineAmount> &outgoingFlows);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

private:

    void deserializeFromBytes(BytesShared buffer);

private:

    map<NodeUUID, TrustLineAmount> mOutgoingFlows;
};


#endif //GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONFROMSOURCEMESSAGE_H
