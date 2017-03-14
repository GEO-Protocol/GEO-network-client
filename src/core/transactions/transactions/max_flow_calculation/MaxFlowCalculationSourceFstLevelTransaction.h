#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationSourceFstLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationSourceSndLevelMessage.h"

class MaxFlowCalculationSourceFstLevelTransaction : public BaseTransaction  {

public:
    typedef shared_ptr<MaxFlowCalculationSourceFstLevelTransaction> Shared;

public:
    MaxFlowCalculationSourceFstLevelTransaction(
        const NodeUUID &nodeUUID,
        MaxFlowCalculationSourceFstLevelMessage::Shared message,
        TrustLinesManager *trustLinesManager,
        Logger *logger);

    MaxFlowCalculationSourceFstLevelMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    MaxFlowCalculationSourceFstLevelMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLog;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELTRANSACTION_H
