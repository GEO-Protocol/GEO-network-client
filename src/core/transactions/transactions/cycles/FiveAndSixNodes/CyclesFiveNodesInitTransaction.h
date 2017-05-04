#ifndef GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H

#include "base/CyclesBaseFiveSixNodesInitTransaction.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesFiveNodesInBetweenMessage.hpp"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesBoundaryMessage.hpp"
#include "../../../../paths/lib/Path.h"



class CyclesFiveNodesInitTransaction :
    public CyclesBaseFiveSixNodesInitTransaction{

public:
    CyclesFiveNodesInitTransaction(
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger);

    const BaseTransaction::TransactionType transactionType() const;

public:
    mutable LaunchCloseCycleSignal closeCycleSignal;

protected:
    const string logHeader() const;

protected:
    TransactionResult::SharedConst runCollectDataAndSendMessagesStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();
};
#endif //GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H
