#ifndef GEO_NETWORK_CLIENT_GETBLACKLISTTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETBLACKLISTTRANSACTION_H


#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/blacklist/GetBlackListCommand.h"
#include "../../../io/storage/StorageHandler.h"


class GetBlackListTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<GetBlackListTransaction> Shared;

public:
    GetBlackListTransaction(
        NodeUUID &nodeUUID,
        GetBlackListCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger)
    noexcept;

    GetBlackListCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    GetBlackListCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_GETBLACKLISTTRANSACTION_H
