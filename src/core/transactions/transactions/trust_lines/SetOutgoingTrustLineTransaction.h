#ifndef GEO_NETWORK_CLIENT_SETOUTGOINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_SETOUTGOINGTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../interface/commands_interface/commands/trust_lines/SetOutgoingTrustLineCommand.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../topology/cashe/TopologyCacheManager.h"
#include "../../../topology/cashe/MaxFlowCacheManager.h"
#include "../../../crypto/keychain.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineMessage.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineFromGatewayMessage.h"
#include "../../../network/messages/trust_lines/TrustLineConfirmationMessage.h"
#include "InitialAuditSourceTransaction.h"
#include "../../../subsystems_controller/SubsystemsController.h"
#include "../../../interface/visual_interface/interface/VisualInterface.h"
#include "../../../interface/visual_interface/visual/VisualResult.h"


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
    public BaseTransaction {

public:
    typedef shared_ptr<SetOutgoingTrustLineTransaction> Shared;

public:
    SetOutgoingTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetOutgoingTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        SubsystemsController *subsystemsController,
        Keystore *keystore,
        VisualInterface *visualInterface,
        bool iAmGateway,
        Logger &logger)
        noexcept;

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        Initialisation = 1,
        ResponseProcessing = 2,
    };

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultForbiddenRun();

    TransactionResult::SharedConst resultProtocolError();

protected: // trust lines history shortcuts
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

protected: // log
    const string logHeader() const
        noexcept;

private:
    TransactionResult::SharedConst runInitialisationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

private:
    static const uint32_t kWaitMillisecondsForResponse = 60000;

protected:
    SetOutgoingTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;
    SubsystemsController *mSubsystemsController;
    Keystore *mKeysStore;
    VisualInterface *mVisualInterface;
    bool mIAmGateway;

    TrustLine::ConstShared mPreviousTL = nullptr;
    TrustLinesManager::TrustLineOperationResult mOperationResult = TrustLinesManager::NoChanges;
};


#endif //GEO_NETWORK_CLIENT_SETOUTGOINGTRUSTLINETRANSACTION_H
