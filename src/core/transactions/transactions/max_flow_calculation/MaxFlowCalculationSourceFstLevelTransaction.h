//
// Created by mc on 16.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/incoming/max_flow_calculation/MaxFlowCalculationSourceFstLevelInMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/MaxFlowCalculationSourceFstLevelOutMessage.h"
#include "../../scheduler/TransactionsScheduler.h"

class MaxFlowCalculationSourceFstLevelTransaction : public BaseTransaction  {

public:
    typedef shared_ptr<MaxFlowCalculationSourceFstLevelTransaction> Shared;

public:
    MaxFlowCalculationSourceFstLevelTransaction(
        NodeUUID &nodeUUID,
        MaxFlowCalculationSourceFstLevelInMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    MaxFlowCalculationSourceFstLevelTransaction(
        BytesShared buffer,
    TrustLinesManager *manager);

    MaxFlowCalculationSourceFstLevelInMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    MaxFlowCalculationSourceFstLevelInMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLog;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELTRANSACTION_H
