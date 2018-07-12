#ifndef GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINETRANSACTION_H

#include "base/BaseTrustLineTransaction.h"
#include "../../../topology/manager/TopologyTrustLinesManager.h"
#include "../../../topology/cashe/TopologyCacheManager.h"
#include "../../../topology/cashe/MaxFlowCacheManager.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineMessage.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineFromGatewayMessage.h"
#include "../../../network/messages/trust_lines/TrustLineConfirmationMessage.h"
#include "../../../subsystems_controller/SubsystemsController.h"
#include "../../../interface/visual_interface/interface/VisualInterface.h"
#include "../../../interface/visual_interface/visual/VisualResult.h"


/**
 * This transaction is used to create/update/close the incoming trust line from the remote contractor.
 *
 * In case if it would be launched against new one contractor with non zero amount -
 * then new one incoming trust line would be created.
 *
 * In case if it would be launched against already present contractor, and transaction amount would not be zero -
 * then incoming trust line to this contractor would be updated to the new amount.
 *
 * In case if it would be launched against already present contractor, and transaction amount WOULD BE zero -
 * then incoming trust line to this contractor would be closed.
 */
class SetIncomingTrustLineTransaction:
    public BaseTrustLineTransaction {

public:
    typedef shared_ptr<SetIncomingTrustLineTransaction> Shared;

public:
    SetIncomingTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetIncomingTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        TopologyTrustLinesManager *topologyTrustLinesManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        SubsystemsController *subsystemsController,
        Keystore *keystore,
        VisualInterface *visualInterface,
        bool iAmGateway,
        Logger &logger)
        noexcept;

    SetIncomingTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetIncomingTrustLineFromGatewayMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        TopologyTrustLinesManager *topologyTrustLinesManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        SubsystemsController *subsystemsController,
        Keystore *keystore,
        VisualInterface *visualInterface,
        bool iAmGateway,
        Logger &logger)
    noexcept;

    SetIncomingTrustLineTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        Logger &logger);

    TransactionResult::SharedConst run();

protected: // trust lines history shortcuts
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

    const string logHeader() const
        noexcept;

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runReceiveAuditStage();

    TransactionResult::SharedConst runRecoveryStage();

    pair<BytesShared, size_t> serializeToBytes() const;

protected:
    TrustLineAmount mAmount;
    TopologyTrustLinesManager *mTopologyTrustLinesManager;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;
    SubsystemsController *mSubsystemsController;
    VisualInterface *mVisualInterface;
    bool mIAmGateway;
    bool mSenderIsGateway;
};


#endif //GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINETRANSACTION_H
