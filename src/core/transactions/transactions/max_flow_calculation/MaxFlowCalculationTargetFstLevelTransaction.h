#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/incoming/max_flow_calculation/MaxFlowCalculationTargetFstLevelInMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/MaxFlowCalculationTargetFstLevelOutMessage.h"

class MaxFlowCalculationTargetFstLevelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationTargetFstLevelTransaction> Shared;

public:
    MaxFlowCalculationTargetFstLevelTransaction(
        const NodeUUID &nodeUUID,
        MaxFlowCalculationTargetFstLevelInMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    MaxFlowCalculationTargetFstLevelInMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    MaxFlowCalculationTargetFstLevelInMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELTRANSACTION_H
