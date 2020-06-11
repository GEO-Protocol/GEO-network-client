#ifndef GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H

#include "base/BaseTrustLineTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines/CloseIncomingTrustLineCommand.h"
#include "../../../network/messages/trust_lines/AuditMessage.h"
#include "../../../topology/manager/TopologyTrustLinesManager.h"
#include "../../../topology/cache/TopologyCacheManager.h"
#include "../../../topology/cache/MaxFlowCacheManager.h"
#include "../../../interface/events_interface/interface/EventsInterfaceManager.h"
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
        EventsInterfaceManager *eventsInterfaceManager,
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

    TransactionResult::SharedConst runAuditPendingStage();

    TransactionResult::SharedConst runResponseProcessingStage();

    TransactionResult::SharedConst runContractorPendingStage();

    TransactionResult::SharedConst initializeAudit();

private:
    CloseIncomingTrustLineCommand::Shared mCommand;
    TopologyTrustLinesManager *mTopologyTrustLinesManager;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;
    EventsInterfaceManager *mEventsInterfaceManager;
    SubsystemsController *mSubsystemsController;

    uint16_t mCountSendingAttempts;
    uint16_t mCountPendingAttempts;
    uint16_t mCountContractorPendingAttempts;

    TrustLineAmount mPreviousIncomingAmount;
    TrustLine::TrustLineState mPreviousState;
};


#endif //GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H
