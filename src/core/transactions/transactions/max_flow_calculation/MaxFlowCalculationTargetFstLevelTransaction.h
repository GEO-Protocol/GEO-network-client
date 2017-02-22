//
// Created by mc on 16.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELTRANSACTION_H

#include "MaxFlowCalculationTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/incoming/max_flow_calculation/MaxFlowCalculationTargetFstLevelInMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/MaxFlowCalculationTargetFstLevelOutMessage.h"
#include "../../scheduler/TransactionsScheduler.h"

class MaxFlowCalculationTargetFstLevelTransaction : public MaxFlowCalculationTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationTargetFstLevelTransaction> Shared;

public:
    MaxFlowCalculationTargetFstLevelTransaction(
        NodeUUID &nodeUUID,
        MaxFlowCalculationTargetFstLevelInMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    MaxFlowCalculationTargetFstLevelTransaction(
        BytesShared buffer,
        TrustLinesManager *manager);

    MaxFlowCalculationTargetFstLevelInMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    MaxFlowCalculationTargetFstLevelInMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELTRANSACTION_H
