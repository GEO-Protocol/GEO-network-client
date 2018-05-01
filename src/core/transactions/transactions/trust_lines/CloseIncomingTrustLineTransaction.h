#ifndef GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../interface/commands_interface/commands/trust_lines/CloseIncomingTrustLineCommand.h"
#include "../../../network/messages/trust_lines/CloseOutgoingTrustLineMessage.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../topology/manager/TopologyTrustLinesManager.h"
#include "../../../topology/cashe/TopologyCacheManager.h"
#include "../../../topology/cashe/MaxFlowCacheManager.h"
#include "../../../subsystems_controller/SubsystemsController.h"

class CloseIncomingTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<CloseIncomingTrustLineTransaction> Shared;

public:
    CloseIncomingTrustLineTransaction(
        const NodeUUID &nodeUUID,
        CloseIncomingTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        TopologyTrustLinesManager *topologyTrustLinesManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        SubsystemsController *subsystemsController,
        Logger &logger)
    noexcept;

    TransactionResult::SharedConst run();

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

protected:
    CloseIncomingTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    TopologyTrustLinesManager *mTopologyTrustLinesManager;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;
    SubsystemsController *mSubsystemsController;
};


#endif //GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H
