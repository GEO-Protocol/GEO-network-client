#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSOBSERVER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSOBSERVER_H

#include "../../common/Types.h"

#include "../transactions/BaseTransaction.h"
#include "../scheduler/TransactionsScheduler.h"

#include <map>

using namespace std;

class TransactionsObserver {

public:
    TransactionsObserver(
        TransactionsScheduler *scheduler);

    ~TransactionsObserver();

    const bool isSameTypeTransactionExist(
        BaseTransaction::Shared transaction) const;

private:
    TransactionsScheduler *mScheduler;

};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSOBSERVER_H
