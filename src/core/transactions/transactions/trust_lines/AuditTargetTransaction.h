#ifndef GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H
#define GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H

#include "base/BaseTrustLineTransaction.h"

#include "../../../topology/manager/TopologyTrustLinesManager.h"
#include "../../../topology/cache/TopologyCacheManager.h"
#include "../../../topology/cache/MaxFlowCacheManager.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"

class AuditTargetTransaction : public BaseTrustLineTransaction {

public:
    typedef shared_ptr<AuditTargetTransaction> Shared;

public:
    AuditTargetTransaction(
        AuditMessage::Shared message,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TopologyTrustLinesManager *topologyTrustLinesManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        FeaturesManager *featuresManager,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    void setIncomingTrustLineAmount(
        IOTransaction::Shared ioTransaction);

    void closeOutgoingTrustLine(
        IOTransaction::Shared ioTransaction);

    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

private:
    string mSenderIncomingIP;
    vector<BaseAddress::Shared> mContractorAddresses;

    AuditMessage::Shared mMessage;
    TopologyTrustLinesManager *mTopologyTrustLinesManager;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;

    TrustLineAmount mPreviousIncomingAmount;
    TrustLineAmount mPreviousOutgoingAmount;
    TrustLine::TrustLineState mPreviousState;
};


#endif //GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H
