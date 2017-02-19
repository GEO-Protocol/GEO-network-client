//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONFROMSOURCETRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONFROMSOURCETRANSACTION_H

#include "MaxFlowCalculationTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/incoming/max_flow_calculation/ResultMaxFlowCalculationFromSourceMessage.h"
#include "../../scheduler/TransactionsScheduler.h"

class ReceiveResultMaxFlowCalculationFromSourceTransaction : public MaxFlowCalculationTransaction {

public:
    typedef shared_ptr<ReceiveResultMaxFlowCalculationFromSourceTransaction> Shared;

public:
    ReceiveResultMaxFlowCalculationFromSourceTransaction(
        NodeUUID &nodeUUID,
        ResultMaxFlowCalculationFromSourceMessage::Shared message,
        TrustLinesManager *manager,
    Logger *logger);

    ReceiveResultMaxFlowCalculationFromSourceTransaction(
        BytesShared buffer,
    TrustLinesManager *manager);

    ResultMaxFlowCalculationFromSourceMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    ResultMaxFlowCalculationFromSourceMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONFROMSOURCETRANSACTION_H
