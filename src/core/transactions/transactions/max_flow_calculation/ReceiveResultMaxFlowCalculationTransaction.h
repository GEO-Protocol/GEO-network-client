//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../../max_flow_calculation/MaxFlowCalculationTrustLine.h"
#include "../../../network/messages/incoming/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../scheduler/TransactionsScheduler.h"

class ReceiveResultMaxFlowCalculationTransaction : public BaseTransaction {

public:
    typedef shared_ptr<ReceiveResultMaxFlowCalculationTransaction> Shared;

public:
    ReceiveResultMaxFlowCalculationTransaction(
        NodeUUID &nodeUUID,
        ResultMaxFlowCalculationMessage::Shared message,
        TrustLinesManager *manager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        Logger *logger);

    ReceiveResultMaxFlowCalculationTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager);

    ResultMaxFlowCalculationMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    ResultMaxFlowCalculationMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONTRANSACTION_H
