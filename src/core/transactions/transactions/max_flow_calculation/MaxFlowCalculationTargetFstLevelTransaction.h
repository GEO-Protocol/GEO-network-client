#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationTargetFstLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationTargetSndLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationGatewayMessage.h"

class MaxFlowCalculationTargetFstLevelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationTargetFstLevelTransaction> Shared;

public:
    MaxFlowCalculationTargetFstLevelTransaction(
        const NodeUUID &nodeUUID,
        MaxFlowCalculationTargetFstLevelMessage::Shared message,
        TrustLinesManager *manager,
        Logger &logger,
        bool iAmGateway);

    MaxFlowCalculationTargetFstLevelMessage::Shared message() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    MaxFlowCalculationTargetFstLevelMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    bool mIAmGateway;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELTRANSACTION_H
