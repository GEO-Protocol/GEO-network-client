#ifndef GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../interface/commands_interface/commands/trust_lines/CloseIncomingTrustLineCommand.h"
#include "../../../network/messages/trust_lines/CloseOutgoingTrustLineMessage.h"
#include "../../../io/storage/StorageHandler.h"
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
    SubsystemsController *mSubsystemsController;
};


#endif //GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINETRANSACTION_H
