#ifndef GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H

#include "../result/MessageResult.h"
#include "../SenderMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

class ResultMaxFlowCalculationMessage : public SenderMessage {

public:
    typedef shared_ptr<ResultMaxFlowCalculationMessage> Shared;

public:

    ResultMaxFlowCalculationMessage(
            const NodeUUID& senderUUID,
            map<NodeUUID, TrustLineAmount> &outgoingFlows,
            map<NodeUUID, TrustLineAmount> &incomingFlows);

    ResultMaxFlowCalculationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    const map<NodeUUID, TrustLineAmount> outgoingFlows() const;

    const map<NodeUUID, TrustLineAmount> incomingFlows() const;

    const bool isMaxFlowCalculationResponseMessage() const;

private:

    void deserializeFromBytes(
        BytesShared buffer);

private:
    map<NodeUUID, TrustLineAmount> mOutgoingFlows;
    map<NodeUUID, TrustLineAmount> mIncomingFlows;

};


#endif //GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H
