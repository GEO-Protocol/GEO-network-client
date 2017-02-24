#ifndef GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H

#include "MaxFlowCalculationTransaction.h"

#include "../../../common/Types.h"
#include "../../../common/NodeUUID.h"

#include "../../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"
#include "../../../network/messages/outgoing/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../../../network/messages/outgoing/max_flow_calculation/SendMaxFlowCalculationSourceFstLevelMessage.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../../logger/Logger.h"

#include <memory>
#include <utility>
#include <stdint.h>

class InitiateMaxFlowCalculationTransaction : public MaxFlowCalculationTransaction {

public:
    typedef shared_ptr<InitiateMaxFlowCalculationTransaction> Shared;

public:
    InitiateMaxFlowCalculationTransaction(
            const NodeUUID &nodeUUID,
            InitiateMaxFlowCalculationCommand::Shared command,
            TrustLinesManager *trustLinesManager,
            MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
            Logger *logger);

    InitiateMaxFlowCalculationTransaction(
            BytesShared buffer,
            TrustLinesManager *trustLinesManager,
            MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager);

    InitiateMaxFlowCalculationCommand::Shared command() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
            BytesShared buffer);

    void sendMessageToRemoteNode();

    void sendMessageOnFirstLevel();

    TransactionResult::SharedConst waitingForResponseState();

private:
    InitiateMaxFlowCalculationCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
