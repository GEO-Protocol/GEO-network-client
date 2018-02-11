#ifndef GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../network/messages/trust_lines/CloseOutgoingTrustLineMessage.h"
#include "../../../network/messages/base/transaction/ConfirmationMessage.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationNodeCacheManager.h"

class CloseOutgoingTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<CloseOutgoingTrustLineTransaction> Shared;

public:
    CloseOutgoingTrustLineTransaction(
        const NodeUUID &nodeUUID,
        CloseOutgoingTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
        Logger &logger)
        noexcept;

    TransactionResult::SharedConst run();

protected:
    TransactionResult::SharedConst resultDone();

protected: // trust lines history shortcuts
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

protected: // log
    const string logHeader() const
        noexcept;

protected:
    CloseOutgoingTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    MaxFlowCalculationNodeCacheManager *mMaxFlowCalculationNodeCacheManager;
};


#endif //GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINETRANSACTION_H
