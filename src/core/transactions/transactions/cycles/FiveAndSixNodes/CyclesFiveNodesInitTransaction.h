#ifndef GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H

#include "base/CyclesBaseFiveSixNodesInitTransaction.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CycleFiveNodesInBetweenMessage.hpp"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesBoundaryMessage.hpp"

class CyclesFiveNodesInitTransaction :
    public CyclesBaseFiveSixNodesInitTransaction{

public:
    CyclesFiveNodesInitTransaction(
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        Logger *logger);

    const BaseTransaction::TransactionType transactionType() const;

protected:
    TransactionResult::SharedConst runCollectDataAndSendMessagesStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();
};
#endif //GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H
