#ifndef GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H
#define GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H


#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/blacklist/AddNodeToBlackListCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/trust_lines/CloseOutgoingTrustLineMessage.h"
#include "../../../subsystems_controller/SubsystemsController.h"


class AddNodeToBlackListTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<AddNodeToBlackListTransaction> Shared;

public:
    AddNodeToBlackListTransaction(
        NodeUUID &nodeUUID,
        AddNodeToBlackListCommand::Shared command,
        StorageHandler *storageHandler,
        TrustLinesManager *trustLinesManager,
        SubsystemsController *subsystemsController,
        Logger &logger);

    AddNodeToBlackListCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultForbiddenRun();

    TransactionResult::SharedConst resultProtocolError();

protected:
    void populateHistory(
        IOTransaction::Shared ioTransaction);

protected:
    const string logHeader() const;

private:
    AddNodeToBlackListCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
    TrustLinesManager *mTrustLinesManager;
    SubsystemsController *mSubsystemsController;
};

#endif //GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H
