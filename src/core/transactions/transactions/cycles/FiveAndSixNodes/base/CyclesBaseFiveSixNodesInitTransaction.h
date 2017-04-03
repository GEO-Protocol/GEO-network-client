#ifndef GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H
#include "../../../base/UniqueTransaction.h"
#include "../../../../../trust_lines/manager/TrustLinesManager.h"

class CyclesBaseFiveSixNodesInitTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<CyclesBaseFiveSixNodesInitTransaction> Shared;
    typedef pair<vector<NodeUUID>, TrustLineBalance> MapValuesType;
    typedef multimap<NodeUUID, MapValuesType> CycleMap;
    typedef CycleMap::iterator mapIter;
    typedef vector<MapValuesType> ResultVector;
public:
    CyclesBaseFiveSixNodesInitTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager,
        Logger *logger);

    CyclesBaseFiveSixNodesInitTransaction(TransactionsScheduler *scheduler);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes() const{};
    void deserializeFromBytes(
        BytesShared buffer){};
protected:
    enum Stages {
        CollectDataAndSendMessage = 1,
        ParseMessageAndCreateCycles
    };

    TrustLinesManager *mTrustLinesManager;
    Logger *mlogger;

protected:
    virtual TransactionResult::SharedConst runCollectDataAndSendMessagesStage() = 0;
    virtual TransactionResult::SharedConst runParseMessageAndCreateCyclesStage() = 0;

protected:
    const uint16_t mWaitingForResponseTime = 5000; //msec
    const uint16_t kMaxRequestsCount = 5;
};

#endif //GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H
