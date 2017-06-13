#ifndef GEO_NETWORK_CLIENT_UPDATEROUTINGTABLES_H
#define GEO_NETWORK_CLIENT_UPDATEROUTINGTABLES_H

#include "../base/BaseTransaction.h"
#include "../../../common/NodeUUID.h"
#include "../../../interface/commands_interface/commands/routing_tables/UpdateRoutingTablesCommand.h"
#include "../../../network/messages/routing_tables/CRC32Rt2ResponseMessage.h"
#include "../../../network/messages/routing_tables/CRC32Rt2RequestMessage.h"
#include "../../../network/messages/routing_tables/CRC32ThirdLevelResponseMessage.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "NeighborsCollectingTransaction.h"

#include <vector>
#include <map>


using namespace std;

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
        Logger &logger)
    noexcept;

    enum Stages {
        askNeighborsForTopologyChangingStage = 1,
        checkFirstAndSecondLevelCRC32SumForNeighborStage,
        checkFirstLevelCRC32SumForNeighborStage,
        checkSecondLevelCRC32SumForNeighborStage,
        UpdateRT2Stage
    };

    UpdateRoutingTablesCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    TransactionResult::SharedConst askNeighborsForTopologyChanging();
    TransactionResult::SharedConst checkFirstAndSecondCRC32rt2Sum();
    TransactionResult::SharedConst checkFirstCRC32rt2Sum();
    TransactionResult::SharedConst checkSecondCRC32rt2Sum();
    TransactionResult::SharedConst updateRougtingTables();

protected:
    const string logHeader() const;

private:
    UpdateRoutingTablesCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    vector<pair<NodeUUID, uint8_t>> mNodesToUpdate;
    map<NodeUUID, NodeUUID> mPathMap;

};
#endif //GEO_NETWORK_CLIENT_UPDATEROUTINGTABLES_H
