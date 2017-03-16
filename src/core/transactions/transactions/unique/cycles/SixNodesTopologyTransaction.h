#ifndef GEO_NETWORK_CLIENT_SIXNODESTOPOLOGYTRANSACTIONS_H
#define GEO_NETWORK_CLIENT_SIXNODESTOPOLOGYTRANSACTIONS_H

#include "GetTopologyAndBalancesTransaction.h"

class SixNodesTopologyTransaction : public GetTopologyAndBalancesTransaction{

public:
    SixNodesTopologyTransaction(
            const TransactionType type,
            const NodeUUID &nodeUUID,
            TransactionsScheduler *scheduler,
            TrustLinesManager *manager,
            Logger *logger);

    SixNodesTopologyTransaction(const TransactionType type,
                                 const NodeUUID &nodeUUID,
                                 InBetweenNodeTopologyMessage::Shared message,
                                 TransactionsScheduler *scheduler,
                                 TrustLinesManager *manager,
                                 Logger *logger);

    const TransactionType transactionType() const;
    InBetweenNodeTopologyMessage::CycleTypeID cycleType();
private:
    void sendFirstLevelNodeMessage();
    const uint8_t mMaxDepth = 2;
};
#endif //GEO_NETWORK_CLIENT_SIXNODESTOPOLOGYTRANSACTIONS_H
