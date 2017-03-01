#ifndef GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../../network/messages/outgoing/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/SendMaxFlowCalculationSourceFstLevelMessage.h"

#include <set>

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

    InitiateMaxFlowCalculationCommand::Shared command() const;

    TransactionResult::SharedConst run();

private:

    void sendMessageToRemoteNode();

    void sendMessageOnFirstLevel();

    TrustLineAmount calculateMaxFlow(const NodeUUID& nodeUUID);

    TrustLineAmount calculateOneNode(
        const NodeUUID& nodeUUID,
        const TrustLineAmount& currentFlow,
        byte level,
        const NodeUUID& targetUUID,
        const NodeUUID& sourceUUID,
        set<NodeUUID> forbiddenUUIDs);

private:

    static const byte kMaxFlowLength=6;

private:

    InitiateMaxFlowCalculationCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
