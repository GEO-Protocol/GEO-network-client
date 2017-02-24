//
// Created by mc on 15.02.17.
//

#ifndef GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONTRANSACTION_H

#include "MaxFlowCalculationTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/incoming/max_flow_calculation/ReceiveMaxFlowCalculationOnTargetMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/SendResultMaxFlowCalculationFromTargetMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/SendMaxFlowCalculationTargetFstLevelMessage.h"
#include "../../scheduler/TransactionsScheduler.h"

class ReceiveMaxFlowCalculationOnTargetTransaction : public MaxFlowCalculationTransaction {

public:
    typedef shared_ptr<ReceiveMaxFlowCalculationOnTargetTransaction> Shared;

public:
    ReceiveMaxFlowCalculationOnTargetTransaction(
            const NodeUUID &nodeUUID,
            ReceiveMaxFlowCalculationOnTargetMessage::Shared message,
            TrustLinesManager *manager,
            Logger *logger);

    ReceiveMaxFlowCalculationOnTargetTransaction(
            BytesShared buffer,
            TrustLinesManager *manager);

    ReceiveMaxFlowCalculationOnTargetMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
            BytesShared buffer);

    void sendMessagesOnFirstLevel();

    void sendResultToInitiator();

private:
    ReceiveMaxFlowCalculationOnTargetMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLog;
};


#endif //GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONTRANSACTION_H
