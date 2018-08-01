#ifndef GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H
#define GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H


#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/blacklist/AddNodeToBlackListCommand.h"
#include "../../../equivalents/EquivalentsSubsystemsRouter.h"
#include "../trust_lines/CloseIncomingTrustLineTransaction.h"
#include "../../../subsystems_controller/SubsystemsController.h"
#include "../../../subsystems_controller/TrustLinesInfluenceController.h"
#include "../../../crypto/keychain.h"


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
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Keystore *keystore,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultForbiddenRun();

    TransactionResult::SharedConst resultProtocolError();

protected:
    const string logHeader() const;

private:
    AddNodeToBlackListCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
    SubsystemsController *mSubsystemsController;
    TrustLinesInfluenceController *mTrustLinesInfluenceController;
    Keystore *mKeysStore;
};

#endif //GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H
