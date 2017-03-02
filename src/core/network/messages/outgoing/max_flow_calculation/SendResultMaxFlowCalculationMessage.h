#ifndef GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONMESSAGE_H

#include "../../result/MessageResult.h"
#include "../../SenderMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class SendResultMaxFlowCalculationMessage : public SenderMessage {

public:
    typedef shared_ptr<SendResultMaxFlowCalculationMessage> Shared;

public:

    SendResultMaxFlowCalculationMessage(
        const NodeUUID& senderUUID,
        map<NodeUUID, TrustLineAmount> &outgoingFlows,
        map<NodeUUID, TrustLineAmount> &incomingFlows);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    const bool isMaxFlowCalculationResponseMessage() const;

private:

    void deserializeFromBytes(BytesShared buffer);

private:

    map<NodeUUID, TrustLineAmount> mOutgoingFlows;
    map<NodeUUID, TrustLineAmount> mIncomingFlows;
};


#endif //GEO_NETWORK_CLIENT_SENDRESULTMAXFLOWCALCULATIONMESSAGE_H
