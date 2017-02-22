//
// Created by mc on 15.02.17.
//

#ifndef GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONFROMTARGETTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONFROMTARGETTRANSACTION_H

#include "MaxFlowCalculationTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/incoming/max_flow_calculation/ResultMaxFlowCalculationFromTargetMessage.h"
#include "../../scheduler/TransactionsScheduler.h"

class ReceiveResultMaxFlowCalculationFromTargetTransaction : public MaxFlowCalculationTransaction {

public:
    typedef shared_ptr<ReceiveResultMaxFlowCalculationFromTargetTransaction> Shared;

public:
    ReceiveResultMaxFlowCalculationFromTargetTransaction(
        NodeUUID &nodeUUID,
        ResultMaxFlowCalculationFromTargetMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    ReceiveResultMaxFlowCalculationFromTargetTransaction(
        BytesShared buffer,
        TrustLinesManager *manager);

    ResultMaxFlowCalculationFromTargetMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    ResultMaxFlowCalculationFromTargetMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_RECEIVERESULTMAXFLOWCALCULATIONFROMTARGETTRANSACTION_H
