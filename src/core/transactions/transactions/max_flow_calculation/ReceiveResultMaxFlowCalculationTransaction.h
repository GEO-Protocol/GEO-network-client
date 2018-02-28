#ifndef GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../topology/manager/TopologyTrustLinesManager.h"
#include "../../../topology/TopologyTrustLine.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationGatewayMessage.h"

class ReceiveResultMaxFlowCalculationTransaction : public BaseTransaction {

public:
    typedef shared_ptr<ReceiveResultMaxFlowCalculationTransaction> Shared;

public:
    ReceiveResultMaxFlowCalculationTransaction(
        NodeUUID &nodeUUID,
        ResultMaxFlowCalculationMessage::Shared message,
        TrustLinesManager *trustLinesManager,
        TopologyTrustLinesManager *topologyTrustLineManager,
        Logger &logger);

    ReceiveResultMaxFlowCalculationTransaction(
        NodeUUID &nodeUUID,
        ResultMaxFlowCalculationGatewayMessage::Shared message,
        TrustLinesManager *trustLinesManager,
        TopologyTrustLinesManager *topologyTrustLineManager,
        Logger &logger);

    ResultMaxFlowCalculationMessage::Shared message() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    ResultMaxFlowCalculationMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    TopologyTrustLinesManager *mTopologyTrustLineManager;
    bool mSenderIsGateway;
};


#endif //GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONTRANSACTION_H
