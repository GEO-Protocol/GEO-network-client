#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H

#include "../transactions/state/TransactionState.h"
#include "../transactions/BaseTransaction.h"

#include <boost/asio.hpp>

#include <map>

using namespace std;

namespace as = boost::asio;

class TransactionsManager;
class TransactionsScheduler {
    friend class TransactionsManager;

private:
    as::io_service &mIOService;
    map<BaseTransaction::Shared, TransactionState::SharedConst> mTransactions;

private:
    TransactionsScheduler(
            as::io_service &IOService);

private:
    void addTransaction(BaseTransaction::Shared transaction);

};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULER_H
