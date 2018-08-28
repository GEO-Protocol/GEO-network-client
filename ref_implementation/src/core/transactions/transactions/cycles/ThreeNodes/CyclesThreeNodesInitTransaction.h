/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../cycles/CyclesManager.h"
#include "../../../../cycles/RoutingTableManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../paths/lib/Path.h"
#include "../../../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesResponseMessage.h"

#include <set>


class CyclesThreeNodesInitTransaction :
    public BaseTransaction {

public:
    CyclesThreeNodesInitTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        TrustLinesManager *manager,
        RoutingTableManager *roughtingTable,
        CyclesManager *cyclesManager,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        CollectDataAndSendMessage = 1,
        ParseMessageAndCreateCycles
    };

    TransactionResult::SharedConst runCollectDataAndSendMessageStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();

protected:
    const string logHeader() const;
    set<NodeUUID> getNeighborsWithContractor();

protected:
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    CyclesManager *mCyclesManager;
    StorageHandler *mStorageHandler;
    RoutingTableManager *mRoughtingTable;
};

#endif //GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H
