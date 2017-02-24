//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/incoming/max_flow_calculation/MaxFlowCalculationTargetSndLevelInMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/SendResultMaxFlowCalculationFromTargetMessage.h"
#include "../../scheduler/TransactionsScheduler.h"

class MaxFlowCalculationTargetSndLevelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationTargetSndLevelTransaction> Shared;

public:
    MaxFlowCalculationTargetSndLevelTransaction(
        NodeUUID &nodeUUID,
        MaxFlowCalculationTargetSndLevelInMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    MaxFlowCalculationTargetSndLevelTransaction(
        BytesShared buffer,
        TrustLinesManager *manager);

    MaxFlowCalculationTargetSndLevelInMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(BytesShared buffer);

    void sendResultToInitiator();

private:
    MaxFlowCalculationTargetSndLevelInMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H
