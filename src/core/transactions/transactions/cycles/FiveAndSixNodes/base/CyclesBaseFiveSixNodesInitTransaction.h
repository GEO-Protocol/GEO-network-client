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
        const SerializedEquivalent equivalent,
        TrustLinesManager *manager,
        CyclesManager *cyclesManager,
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
    CyclesManager *mCyclesManager;
};

#endif //GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H
