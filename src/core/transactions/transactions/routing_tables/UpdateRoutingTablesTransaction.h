#ifndef GEO_NETWORK_CLIENT_UPDATEROUTINGTABLES_H
#define GEO_NETWORK_CLIENT_UPDATEROUTINGTABLES_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/routing_tables/UpdateRoutingTablesCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "NeighborsCollectingTransaction.h"

class UpdateRoutingTablesTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<UpdateRoutingTablesTransaction> Shared;

public:
    UpdateRoutingTablesTransaction(
        NodeUUID &nodeUUID,
        UpdateRoutingTablesCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger)
    noexcept;

    UpdateRoutingTablesCommand::Shared command() const;

    TransactionResult::SharedConst run();

private:
    UpdateRoutingTablesCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;

};
#endif //GEO_NETWORK_CLIENT_UPDATEROUTINGTABLES_H
