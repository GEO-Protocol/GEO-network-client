#ifndef GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H

#include "../result/MessageResult.h"
#include "../SenderMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

#include <vector>

class ResultMaxFlowCalculationMessage : public SenderMessage {

public:
    typedef shared_ptr<ResultMaxFlowCalculationMessage> Shared;

public:

    ResultMaxFlowCalculationMessage(
            const NodeUUID& senderUUID,
            vector<pair<NodeUUID, TrustLineAmount>> &outgoingFlows,
            vector<pair<NodeUUID, TrustLineAmount>> &incomingFlows);

    ResultMaxFlowCalculationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    const vector<pair<NodeUUID, TrustLineAmount>> outgoingFlows() const;

    const vector<pair<NodeUUID, TrustLineAmount>> incomingFlows() const;

    const bool isMaxFlowCalculationResponseMessage() const;

private:

    void deserializeFromBytes(
        BytesShared buffer);

private:

    typedef uint32_t RecordNumber;
    typedef RecordNumber RecordCount;

private:
    vector<pair<NodeUUID, TrustLineAmount>> mOutgoingFlows;
    vector<pair<NodeUUID, TrustLineAmount>> mIncomingFlows;

};


#endif //GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H
