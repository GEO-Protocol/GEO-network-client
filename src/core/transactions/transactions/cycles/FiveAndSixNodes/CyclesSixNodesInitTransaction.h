#ifndef GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H

#include "base/CyclesBaseFiveSixNodesInitTransaction.h"

class CyclesSixNodesInitTransaction : public CyclesBaseFiveSixNodesInitTransaction{

public:
    CyclesSixNodesInitTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager,
        Logger *logger);

    const BaseTransaction::TransactionType transactionType() const;
    InBetweenNodeTopologyMessage::CycleTypeID cycleType();

protected:
    TransactionResult::SharedConst runCollectDataAndSendMessagesStage();

protected:
    const uint8_t mMaxDepth = 2;
};
#endif //GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H
