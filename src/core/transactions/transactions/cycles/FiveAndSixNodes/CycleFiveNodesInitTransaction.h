#ifndef GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H

#include "base/CyclesBaseFiveSixNodesInitTransaction.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CycleFiveNodesInBetweenMessage.hpp"

class CycleFiveNodesInitTransaction : public CyclesBaseFiveSixNodesInitTransaction{

public:
    CycleFiveNodesInitTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager,
        Logger *logger);

    const BaseTransaction::TransactionType transactionType() const;

protected:
    TransactionResult::SharedConst runCollectDataAndSendMessagesStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();

protected:
    const uint8_t mMaxDepthDebtors = 2;
    const uint8_t mMaxDepthCreditors = 1;
};
#endif //GEO_NETWORK_CLIENT_CYCLESFIVENODESINITTRANSACTION_H
