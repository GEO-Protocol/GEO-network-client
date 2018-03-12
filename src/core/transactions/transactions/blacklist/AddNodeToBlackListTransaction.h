#ifndef GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H
#define GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H


#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/blacklist/AddNodeToBlackListCommand.h"
#include "../../../equivalents/EquivalentsSubsystemsRouter.h"
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
        EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
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
        const SerializedEquivalent equivalent,
        IOTransaction::Shared ioTransaction);

protected:
    const string logHeader() const;

private:
    AddNodeToBlackListCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
    SubsystemsController *mSubsystemsController;
};

#endif //GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H
