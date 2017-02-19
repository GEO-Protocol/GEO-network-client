//
// Created by denis on 13.02.17.
//

#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESTRANSACTION_H

#include "../../base/UniqueTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../network/messages/cycles/GetTopologyAndBalancesMessageInBetweenNode.h"


class GetTopologyAndBalancesTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<GetTopologyAndBalancesTransaction> Shared;

public:
    GetTopologyAndBalancesTransaction(TransactionType type,
                                      NodeUUID &nodeUUID,
                                      TransactionsScheduler *scheduler,
                                      TrustLinesManager *manager,
                                      Logger *logger);
//    GetTopologyAndBalancesTransaction(BaseTransaction::TransactionType type,
//                                      NodeUUID &nodeUUID,
//                                      TransactionsScheduler *scheduler,
//                                      TrustLinesManager *manager,
//                                      Logger *logger
//    );
//    GetTopologyAndBalancesTransaction(TransactionsScheduler *scheduler);
    ~GetTopologyAndBalancesTransaction();
    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();
    void deserializeFromBytes(
            BytesShared buffer);
private:
//    vector<Message> getMessagesToSent();
private:
    const uint16_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

//    uint8_t mMax_depth = 2;
//    NodeUUID &mNodeUUID;
//    TrustLinesManager *mTrustLinesManager;
//    Logger *mlogger;

};



#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESTRANSACTION_H
