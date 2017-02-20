//
// Created by denis on 13.02.17.
//

#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESTRANSACTION_H

#include "../../base/UniqueTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../network/messages/cycles/InBetweenNodeTopologyMessage.h"
#include "../../../../network/messages/cycles/BoundaryNodeTopolodyMessage.h"

class GetTopologyAndBalancesTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<GetTopologyAndBalancesTransaction> Shared;

public:
    GetTopologyAndBalancesTransaction(TransactionType type,
                                      NodeUUID &nodeUUID,
                                      TransactionsScheduler *scheduler,
                                      TrustLinesManager *manager,
                                      Logger *logger);

    GetTopologyAndBalancesTransaction(TransactionType type,
                                      NodeUUID &nodeUUID,
                                      InBetweenNodeTopologyMessage::Shared message,
                                      TransactionsScheduler *scheduler,
                                      TrustLinesManager *manager,
                                      Logger *logger);

    GetTopologyAndBalancesTransaction(TransactionType type,
                                      NodeUUID &nodeUUID,
                                      BoundaryNodeTopolodyMessage::Shared message,
                                      TransactionsScheduler *scheduler,
                                      TrustLinesManager *manager,
                                      Logger *logger);

    GetTopologyAndBalancesTransaction(TransactionsScheduler *scheduler);
    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();
    void deserializeFromBytes(
            BytesShared buffer);
private:
//    vector<Message> getMessagesToSent();
private:
    const uint16_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    uint8_t mMax_depth = 2;
    TrustLinesManager *mTrustLinesManager;
    Logger *mlogger;
    InBetweenNodeTopologyMessage::Shared mInBetweeenMessage = nullptr;
    BoundaryNodeTopolodyMessage::Shared mBoundaryMessage = nullptr;

};



#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESTRANSACTION_H
