#ifndef GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H

#include "base/CyclesBaseFiveSixNodesInitTransaction.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesBoundaryMessage.hpp"
#include "../../../../paths/lib/Path.h"

class CyclesSixNodesInitTransaction :
        public CyclesBaseFiveSixNodesInitTransaction{

public:
    CyclesSixNodesInitTransaction(
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
#endif //GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H
