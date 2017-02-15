//
// Created by denis on 13.02.17.
//

#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESTRANSACTION_H

#include "../../base/UniqueTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"


class GetTopologyAndBalancesTransaction : public UniqueTransaction {


public:
    typedef shared_ptr<GetTopologyAndBalancesTransaction> Shared;

public:
    GetTopologyAndBalancesTransaction(BaseTransaction::TransactionType type,
                                      NodeUUID &nodeUUID,
                                      TransactionsScheduler *scheduler1
    );
    GetTopologyAndBalancesTransaction(TransactionsScheduler *scheduler);

private:
    const uint16_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

//    OpenTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    Logger *mlogger;
};



#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESTRANSACTION_H
