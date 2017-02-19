//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELTRANSACTION_H

#include "MaxFlowCalculationTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/incoming/max_flow_calculation/MaxFlowCalculationSourceSndLevelInMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/SendResultMaxFlowCalculationFromSourceMessage.h"
#include "../../scheduler/TransactionsScheduler.h"

class MaxFlowCalculationSourceSndLevelTransaction : public MaxFlowCalculationTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationSourceSndLevelTransaction> Shared;

public:
    MaxFlowCalculationSourceSndLevelTransaction(
        NodeUUID &nodeUUID,
        MaxFlowCalculationSourceSndLevelInMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    MaxFlowCalculationSourceSndLevelTransaction(
        BytesShared buffer,
        TrustLinesManager *manager);

    MaxFlowCalculationSourceSndLevelInMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(BytesShared buffer);

    void sendResultToInitiator();

private:
    MaxFlowCalculationSourceSndLevelInMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELTRANSACTION_H
