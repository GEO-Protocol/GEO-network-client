#include "GetTopologyAndBalancesTransaction.h"

GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(BaseTransaction::TransactionType type,
                                                                     NodeUUID &nodeUUID,
                                                                     TransactionsScheduler *scheduler1)
        : UniqueTransaction(type, nodeUUID, scheduler1) {
    cout << "Constructor" << endl;
};

GetTopologyAndBalancesTransaction::GetTopologyAndBalancesTransaction(TransactionsScheduler *scheduler)
        : UniqueTransaction(scheduler) {
    cout << "Constructor scheduler" << endl;
}
