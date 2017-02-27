#ifndef GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"

#include "../../scheduler/TransactionsScheduler.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../../network/messages/outgoing/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/SendMaxFlowCalculationSourceFstLevelMessage.h"

class InitiateMaxFlowCalculationTransaction : public BaseTransaction {

public:
    typedef shared_ptr<InitiateMaxFlowCalculationTransaction> Shared;

public:
    InitiateMaxFlowCalculationTransaction(
            NodeUUID &nodeUUID,
            InitiateMaxFlowCalculationCommand::Shared command,
            TrustLinesManager *manager,
            MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
            Logger *logger);

    InitiateMaxFlowCalculationTransaction(
            BytesShared buffer,
            TrustLinesManager *manager,
            MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager);

    InitiateMaxFlowCalculationCommand::Shared command() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
            BytesShared buffer);

    void sendMessageToRemoteNode();

    void sendMessageOnFirstLevel();

    TrustLineAmount calculateMaxFlow(const NodeUUID& nodeUUID);

    TrustLineAmount calculateOneNode(
        const NodeUUID& nodeUUID,
        const TrustLineAmount& currentFlow,
        int level,
        const NodeUUID& targetUUID,
        const NodeUUID& sourceUUID);

    void testCompare(MaxFlowCalculationTrustLine::Shared a,
                     MaxFlowCalculationTrustLine::Shared b);

private:

    InitiateMaxFlowCalculationCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
