#ifndef GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H

#include "base/BaseTrustLineTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines/CloseIncomingTrustLineCommand.h"
#include "../../../network/messages/trust_lines/AuditMessage.h"
#include "../../../topology/manager/TopologyTrustLinesManager.h"
#include "../../../topology/cache/TopologyCacheManager.h"
#include "../../../topology/cache/MaxFlowCacheManager.h"
#include "../../../interface/events_interface/interface/EventsInterface.h"
#include "../../../subsystems_controller/SubsystemsController.h"

class CloseIncomingTrustLineTransaction : public BaseTrustLineTransaction {

public:
    typedef shared_ptr<CloseIncomingTrustLineTransaction> Shared;

public:
    CloseIncomingTrustLineTransaction(
        CloseIncomingTrustLineCommand::Shared command,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        TopologyTrustLinesManager *topologyTrustLinesManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        SubsystemsController *subsystemsController,
        Keystore *keystore,
        FeaturesManager *featuresManager,
        EventsInterface *eventsInterface,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultForbiddenRun();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultKeysError();

    TransactionResult::SharedConst resultUnexpectedError();

protected:
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

    const string logHeader() const override;

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

private:
    CloseIncomingTrustLineCommand::Shared mCommand;
    TopologyTrustLinesManager *mTopologyTrustLinesManager;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;
    EventsInterface *mEventsInterface;
    SubsystemsController *mSubsystemsController;

    uint16_t mCountSendingAttempts;

    TrustLineAmount mPreviousIncomingAmount;
    TrustLine::TrustLineState mPreviousState;
};


#endif //GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H
