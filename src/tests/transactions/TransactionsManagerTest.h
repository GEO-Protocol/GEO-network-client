#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULERTEST_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULERTEST_H


#include "../../core/transactions/manager/TransactionsManager.h"

class TransactionsManagerTest {

public:
    TransactionsManager *mTransactionsManager;
    Logger *mLog;

public:
    TransactionsManagerTest(
            as::io_service &IOService,
            TrustLinesManager *trustLinesManager,
            ResultsInterface *resultsInterface,
            Logger *logger);

    ~TransactionsManagerTest();

    void runFewTransactionsWithSuccessResultTestCase();

    void runSingleTransactionWithFailureStateAndAfterRelaunchingWithSuccessResultTestCase();

    void runSingleTransactionWhichCrashWhileRunning();

    void runFewTransactionsWhichReturnsFailureStateAndAfterRelaunchingWithSuccessResultTestCase();

    void runAllCases();

    void run();
};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSSCHEDULERTEST_H
