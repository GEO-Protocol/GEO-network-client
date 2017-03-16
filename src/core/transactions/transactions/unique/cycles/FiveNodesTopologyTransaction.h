#ifndef GEO_NETWORK_CLIENT_FIVENODESTOPOLODYRANSACTIONS_H
#define GEO_NETWORK_CLIENT_FIVENODESTOPOLODYRANSACTIONS_H

#include "GetTopologyAndBalancesTransaction.h"

class FiveNodesTopologyTransaction : public GetTopologyAndBalancesTransaction{

public:
    FiveNodesTopologyTransaction(
            const TransactionType type,
            const NodeUUID &nodeUUID,
            TransactionsScheduler *scheduler,
            TrustLinesManager *manager,
            Logger *logger);

    FiveNodesTopologyTransaction(const TransactionType type,
                                      const NodeUUID &nodeUUID,
                                      InBetweenNodeTopologyMessage::Shared message,
                                      TransactionsScheduler *scheduler,
                                      TrustLinesManager *manager,
                                      Logger *logger);

    const BaseTransaction::TransactionType transactionType() const;
    InBetweenNodeTopologyMessage::CycleTypeID cycleType();

private:
    void sendFirstLevelNodeMessage();
    const uint8_t mMaxDepthDebtors = 2;
    const uint8_t mMaxDepthCreditors = 1;
};
#endif //GEO_NETWORK_CLIENT_FIVENODESTOPOLODYRANSACTIONS_H
