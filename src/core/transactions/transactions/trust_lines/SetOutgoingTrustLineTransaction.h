#ifndef GEO_NETWORK_CLIENT_SETOUTGOINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_SETOUTGOINGTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../interface/commands_interface/commands/trust_lines/SetOutgoingTrustLineCommand.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationNodeCacheManager.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineMessage.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineFromGatewayMessage.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
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
    public BaseTransaction {

public:
    typedef shared_ptr<SetOutgoingTrustLineTransaction> Shared;

public:
    SetOutgoingTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetOutgoingTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
        SubsystemsController *subsystemsController,
        bool iAmGateway,
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
    SetOutgoingTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    MaxFlowCalculationNodeCacheManager *mMaxFlowCalculationNodeCacheManager;
    SubsystemsController *mSubsystemsController;
    bool mIAmGateway;
};


#endif //GEO_NETWORK_CLIENT_SETOUTGOINGTRUSTLINETRANSACTION_H
