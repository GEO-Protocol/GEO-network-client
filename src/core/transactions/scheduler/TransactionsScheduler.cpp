#include "TransactionsScheduler.h"

TransactionsScheduler::TransactionsScheduler(as::io_service &IOService) :
        mIOService(IOService) {}

void TransactionsScheduler::addTransaction(BaseTransaction::Shared transaction) {
    mTransactions.insert(make_pair(transaction, nullptr));
}
