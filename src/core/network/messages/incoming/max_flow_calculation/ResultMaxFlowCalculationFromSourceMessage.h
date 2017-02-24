//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONFROMSOURCEMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONFROMSOURCEMESSAGE_H

#include "../../result/MessageResult.h"
#include "../../SenderMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class ResultMaxFlowCalculationFromSourceMessage : public SenderMessage {

public:
    typedef shared_ptr<ResultMaxFlowCalculationFromSourceMessage> Shared;

public:
    ResultMaxFlowCalculationFromSourceMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    static const size_t kRequestedBufferSize();

    static const size_t kRequestedBufferSize(unsigned char* buffer);

    map<NodeUUID, TrustLineAmount> getOutgoingFlows();

private:

    void deserializeFromBytes(
        BytesShared buffer);

private:
    map<NodeUUID, TrustLineAmount> mOutgoingFlows;

};


#endif //GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONFROMSOURCEMESSAGE_H
