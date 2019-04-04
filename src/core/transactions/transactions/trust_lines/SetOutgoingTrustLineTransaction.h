#ifndef GEO_NETWORK_CLIENT_SETOUTGOINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_SETOUTGOINGTRUSTLINETRANSACTION_H

#include "base/BaseTrustLineTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines/SetOutgoingTrustLineCommand.h"
#include "../../../topology/cashe/TopologyCacheManager.h"
#include "../../../topology/cashe/MaxFlowCacheManager.h"
#include "../../../interface/events_interface/interface/EventsInterface.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"
#include "../../../network/messages/trust_lines/AuditMessage.h"
#include "../../../subsystems_controller/SubsystemsController.h"


/**
 * This transaction is used to create/update/close the outgoing trust line for the remote contractor.
 *
 * In case if it would be launched against new one contractor with non zero amount -
 * then new one outgoing trust line would be created.
 *
 * In case if it would be launched against already present contractor, and transaction amount would not be zero -
 * then outgoing trust line to this contractor would be updated to the new amount.
 *
 * In case if it would be launched against already present contractor, and transaction amount WOULD BE zero -
 * then outgoing trust line to this contractor would be closed.
 */
class SetOutgoingTrustLineTransaction:
    public BaseTrustLineTransaction {

public:
    typedef shared_ptr<SetOutgoingTrustLineTransaction> Shared;

public:
    SetOutgoingTrustLineTransaction(
        SetOutgoingTrustLineCommand::Shared command,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        SubsystemsController *subsystemsController,
        Keystore *keystore,
        FeaturesManager *featuresManager,
        EventsInterface *eventsInterface,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger)
        noexcept;

    TransactionResult::SharedConst run();

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultForbiddenRun();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultKeysError();

    TransactionResult::SharedConst resultUnexpectedError();

protected: // trust lines history shortcuts
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

protected: // log
    const string logHeader() const
        noexcept;

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

private:
    SetOutgoingTrustLineCommand::Shared mCommand;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;
    EventsInterface *mEventsInterface;
    SubsystemsController *mSubsystemsController;

    uint16_t mCountSendingAttempts;

    TrustLineAmount mPreviousOutgoingAmount;
    TrustLine::TrustLineState mPreviousState;
    TrustLinesManager::TrustLineOperationResult mOperationResult = TrustLinesManager::NoChanges;
};


#endif //GEO_NETWORK_CLIENT_SETOUTGOINGTRUSTLINETRANSACTION_H
