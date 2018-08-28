/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H

#include "../../../base/BaseTransaction.h"
#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../cycles/CyclesManager.h"

class CyclesBaseFiveSixNodesInitTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<CyclesBaseFiveSixNodesInitTransaction> Shared;
    typedef multimap<NodeUUID, const shared_ptr<vector<NodeUUID>>> CycleMap;

public:
    CyclesBaseFiveSixNodesInitTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        CyclesManager *cyclesManager,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        CollectDataAndSendMessage = 1,
        ParseMessageAndCreateCycles
    };

    virtual TransactionResult::SharedConst runCollectDataAndSendMessagesStage() = 0;
    virtual TransactionResult::SharedConst runParseMessageAndCreateCyclesStage() = 0;
    virtual const string logHeader() const = 0;

protected:
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    CyclesManager *mCyclesManager;
};

#endif //GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H
