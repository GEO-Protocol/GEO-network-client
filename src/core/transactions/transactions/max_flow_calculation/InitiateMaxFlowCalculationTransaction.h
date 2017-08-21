#ifndef GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../../network/messages/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationSourceFstLevelMessage.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"

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
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &logger);

    InitiateMaxFlowCalculationCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        SendRequestForCollectingTopology = 1,
        CalculateMaxTransactionFlow
    };

protected:
    const string logHeader() const;

private:
    void sendMessagesToContractors();

    void sendMessagesOnFirstLevel();

    TrustLineAmount calculateMaxFlow(
        const NodeUUID &contractorUUID);

    void calculateMaxFlowOnOneLevel(
        const NodeUUID& contractorUUID);

    TrustLineAmount calculateOneNode(
        const NodeUUID& nodeUUID,
        const NodeUUID& contractorUUID,
        const TrustLineAmount& currentFlow,
        byte level);

    TransactionResult::SharedConst resultOk(
        vector<pair<NodeUUID, TrustLineAmount>> &maxFlows);

    TransactionResult::SharedConst resultProtocolError();

private:
    static const byte kMaxPathLength = 6;
    static const uint32_t kWaitMilisecondsForCalculatingMaxFlow = 2000;

private:
    InitiateMaxFlowCalculationCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    vector<NodeUUID> mForbiddenNodeUUIDs;
    byte mCurrentPathLength;
    TrustLineAmount mCurrentMaxFlow;
};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
