#ifndef GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINETRANSACTION_H

#include "base/BaseTrustLineTransaction.h"
#include "../../../network/messages/trust_lines/CloseOutgoingTrustLineMessage.h"
#include "../../../topology/cashe/TopologyCacheManager.h"
#include "../../../topology/cashe/MaxFlowCacheManager.h"

class CloseOutgoingTrustLineTransaction : public BaseTrustLineTransaction {

public:
    typedef shared_ptr<CloseOutgoingTrustLineTransaction> Shared;

public:
    CloseOutgoingTrustLineTransaction(
        const NodeUUID &nodeUUID,
        CloseOutgoingTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger)
        noexcept;

    CloseOutgoingTrustLineTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
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
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;
};


#endif //GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINETRANSACTION_H
