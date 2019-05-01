#ifndef GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H

#include "../../../base/BaseTransaction.h"
#include "../../../../../contractors/ContractorsManager.h"
#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../cycles/CyclesManager.h"

class CyclesBaseFiveSixNodesInitTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<CyclesBaseFiveSixNodesInitTransaction> Shared;
    typedef multimap<string, vector<BaseAddress::Shared>> CycleMap;

public:
    CyclesBaseFiveSixNodesInitTransaction(
        const TransactionType type,
        const SerializedEquivalent equivalent,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLinesManager,
        CyclesManager *cyclesManager,
        TailManager *tailManager,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    enum Stages {
        CollectDataAndSendMessage = 1,
        ParseMessageAndCreateCycles
    };

    virtual TransactionResult::SharedConst runCollectDataAndSendMessagesStage() = 0;
    virtual TransactionResult::SharedConst runParseMessageAndCreateCyclesStage() = 0;

protected:
    static const uint16_t mkWaitingForResponseTime = 5000;

protected:
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLinesManager;
    CyclesManager *mCyclesManager;
    TailManager *mTailManager;
};

#endif //GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H
