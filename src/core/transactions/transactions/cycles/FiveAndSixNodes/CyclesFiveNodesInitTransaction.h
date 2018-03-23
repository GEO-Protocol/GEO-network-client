#ifndef GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H

#include "base/CyclesBaseFiveSixNodesInitTransaction.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesFiveNodesInBetweenMessage.hpp"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesFiveNodesBoundaryMessage.hpp"
#include "../../../../paths/lib/Path.h"

class CyclesFiveNodesInitTransaction :
    public CyclesBaseFiveSixNodesInitTransaction{

public:
    CyclesFiveNodesInitTransaction(
        const NodeUUID &nodeUUID,
        const SerializedEquivalent equivalent,
        TrustLinesManager *manager,
        CyclesManager *cyclesManager,
        Logger &logger);

    const BaseTransaction::TransactionType transactionType() const;

protected:
    const string logHeader() const;

protected:
    TransactionResult::SharedConst runCollectDataAndSendMessagesStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();
};
#endif //GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H
