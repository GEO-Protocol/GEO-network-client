#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/incoming/max_flow_calculation/MaxFlowCalculationTargetSndLevelInMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/SendResultMaxFlowCalculationMessage.h"
#include "../../scheduler/TransactionsScheduler.h"

class MaxFlowCalculationTargetSndLevelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationTargetSndLevelTransaction> Shared;

public:
    MaxFlowCalculationTargetSndLevelTransaction(
        const NodeUUID &nodeUUID,
        MaxFlowCalculationTargetSndLevelInMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    MaxFlowCalculationTargetSndLevelInMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:

    void sendResultToInitiator();

private:
    MaxFlowCalculationTargetSndLevelInMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H
