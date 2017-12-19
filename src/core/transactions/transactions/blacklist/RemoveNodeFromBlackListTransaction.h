#ifndef GEO_NETWORK_CLIENT_REMOVENODEFROMBLACKLISTTRANSACTION_H
#define GEO_NETWORK_CLIENT_REMOVENODEFROMBLACKLISTTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/blacklist/RemoveNodeFromBlackListCommand.h"
#include "../../../io/storage/StorageHandler.h"

class RemoveNodeFromBlackListTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<RemoveNodeFromBlackListTransaction> Shared;

public:
    RemoveNodeFromBlackListTransaction(
        NodeUUID &nodeUUID,
        RemoveNodeFromBlackListCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    RemoveNodeFromBlackListCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;
protected:
    RemoveNodeFromBlackListCommand::Shared mCommand;
    StorageHandler *mStorageHandler;

};

#endif //GEO_NETWORK_CLIENT_REMOVENODEFROMBLACKLISTTRANSACTION_H
