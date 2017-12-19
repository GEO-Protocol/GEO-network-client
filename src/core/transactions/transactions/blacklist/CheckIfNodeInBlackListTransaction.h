#ifndef GEO_NETWORK_CLIENT_CheckIfNodeInBlackListTransactionTRANSACTION_H
#define GEO_NETWORK_CLIENT_CheckIfNodeInBlackListTransactionTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/blacklist/CheckIfNodeInBlackListCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"


class CheckIfNodeInBlackListTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<CheckIfNodeInBlackListTransaction> Shared;

public:
    CheckIfNodeInBlackListTransaction(
        NodeUUID &nodeUUID,
        CheckIfNodeInBlackListCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    CheckIfNodeInBlackListCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    CheckIfNodeInBlackListCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
};

#endif //GEO_NETWORK_CLIENT_CheckIfNodeInBlackListTransactionTRANSACTION_H
