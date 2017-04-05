#ifndef GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H

#include "base/CyclesBaseFiveSixNodesInitTransaction.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CycleSixNodesInBetweenMessage.hpp"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CycleSixNodesBoundaryMessage.hpp"

class CyclesSixNodesInitTransaction : public CyclesBaseFiveSixNodesInitTransaction{

public:
    CyclesSixNodesInitTransaction(
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        Logger *logger);

    const BaseTransaction::TransactionType transactionType() const;

protected:
    TransactionResult::SharedConst runCollectDataAndSendMessagesStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();

};
#endif //GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H
