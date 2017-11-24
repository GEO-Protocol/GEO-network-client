#ifndef GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H
#define GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H


#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/blacklist/AddNodeToBlackListCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"


class AddNodeToBlackListTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<AddNodeToBlackListTransaction> Shared;

public:
    AddNodeToBlackListTransaction(
        NodeUUID &nodeUUID,
        AddNodeToBlackListCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    AddNodeToBlackListCommand::Shared command() const;

    TransactionResult::SharedConst run();

private:
    AddNodeToBlackListCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
};
#endif //GEO_NETWORK_CLIENT_ADDNODETOBLACKLISTTRANSACTION_H
