#ifndef GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H
#define GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H

#include "base/BaseTrustLineTransaction.h"

#include "../../../topology/manager/TopologyTrustLinesManager.h"
#include "../../../topology/cashe/TopologyCacheManager.h"
#include "../../../topology/cashe/MaxFlowCacheManager.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"
#include "../../../subsystems_controller/SubsystemsController.h"
#include "../../../interface/visual_interface/interface/VisualInterface.h"
#include "../../../interface/visual_interface/visual/VisualResult.h"

class AuditTargetTransaction : public BaseTrustLineTransaction {

public:
    typedef shared_ptr<AuditTargetTransaction> Shared;

public:
    AuditTargetTransaction(
        const NodeUUID &nodeUUID,
        AuditMessage::Shared message,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TopologyTrustLinesManager *topologyTrustLinesManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        SubsystemsController *subsystemsController,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        VisualInterface *visualInterface,
        Logger &logger);

    TransactionResult::SharedConst run();

protected: // log
    const string logHeader() const;

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
    SubsystemsController *mSubsystemsController;
    VisualInterface *mVisualInterface;

    TrustLineAmount mPreviousIncomingAmount;
    TrustLineAmount mPreviousOutgoingAmount;
    TrustLine::TrustLineState mPreviousState;
};


#endif //GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H
