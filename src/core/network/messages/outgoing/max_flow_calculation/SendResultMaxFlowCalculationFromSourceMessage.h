//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONFROMSOURCEMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONFROMSOURCEMESSAGE_H

#include "../../result/MessageResult.h"
#include "../../SenderMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class SendResultMaxFlowCalculationFromSourceMessage : public SenderMessage {

public:
    typedef shared_ptr<SendResultMaxFlowCalculationFromSourceMessage> Shared;

public:

    SendResultMaxFlowCalculationFromSourceMessage(
        NodeUUID &senderUUID,
        map<NodeUUID, TrustLineAmount> &outgoingFlows);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

private:

    void deserializeFromBytes(BytesShared buffer);

private:

    map<NodeUUID, TrustLineAmount> mOutgoingFlows;
};


#endif //GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONFROMSOURCEMESSAGE_H
