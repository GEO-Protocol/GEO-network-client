#include "TransactionsManagerTest.h"

TransactionsManagerTest::TransactionsManagerTest(
        as::io_service &IOService,
        TrustLinesManager *trustLinesManager,
        ResultsInterface *resultsInterface,
        Logger *logger) :

        mLog(logger){

    mTransactionsManager = new TransactionsManager(IOService, trustLinesManager, resultsInterface, logger);
}

TransactionsManagerTest::~TransactionsManagerTest() {

    if (mTransactionsManager != nullptr) {
        delete mTransactionsManager;
    }
}

void TransactionsManagerTest::runFewTransactionsWithSuccessResultTestCase() {

//    mLog->mFileLogger->writeEmptyLine(1);
//    mLog->logInfo("Transactions manager test",
//                  "_________________________________________________");
//    mLog->logInfo("Transactions manager test",
//                  "Case 1. Run few transactions with success result.");
//    mLog->logInfo("Transactions manager test",
//                  "_________________________________________________");

    CommandUUID command1UUID;
    BaseUserCommand *command1 = new OpenTrustLineCommand(command1UUID, "550e8400-e29b-41d4-a716-446655440000\t200\n");

    CommandUUID command2UUID;
    BaseUserCommand *command2 = new OpenTrustLineCommand(command2UUID, "550e8400-e29b-41d4-a716-446655440000\t200\n");

    mTransactionsManager->processCommand(BaseUserCommand::Shared(command1));
    mTransactionsManager->processCommand(BaseUserCommand::Shared(command2));

}

void TransactionsManagerTest::runSingleTransactionWithFailureStateAndAfterRelaunchingWithSuccessResultTestCase() {

//    mLog->mFileLogger->writeEmptyLine(1);
//    mLog->logInfo("Transactions manager test",
//                  "_________________________________________________");
//    mLog->logInfo("Transactions manager test",
//                  "Case 2. Run single transaction which first returns failure result, relaunch after 15 sec and return success result.");
//    mLog->logInfo("Transactions manager test",
//                  "_________________________________________________");

    CommandUUID command1UUID;
    BaseUserCommand *command1 = new UpdateTrustLineCommand(command1UUID, "550e8400-e29b-41d4-a716-446655440000\t200\n");

    mTransactionsManager->processCommand(BaseUserCommand::Shared(command1));

}

void TransactionsManagerTest::runSingleTransactionWhichCrashWhileRunning() {
//    mLog->mFileLogger->writeEmptyLine(1);
//    mLog->logInfo("Transactions manager test",
//                  "_________________________________________________");
//    mLog->logInfo("Transactions manager test",
//                  "Case 3. Run single transaction which crash while running.");
//    mLog->logInfo("Transactions manager test",
//                  "_________________________________________________");

    CommandUUID command1UUID;
    BaseUserCommand *command1 = new CloseTrustLineCommand(command1UUID, "550e8400-e29b-41d4-a716-446655440000\n");

    mTransactionsManager->processCommand(BaseUserCommand::Shared(command1));
}

void TransactionsManagerTest::runFewTransactionsWhichReturnsFailureStateAndAfterRelaunchingWithSuccessResultTestCase() {
//    mLog->mFileLogger->writeEmptyLine(1);
//    mLog->logInfo("Transactions manager test",
//                  "_________________________________________________");
//    mLog->logInfo("Transactions manager test",
//                  "Case 4. Run few transactions which first returns failure result, relaunching after 15 and 25 sec and returns success result.");
//    mLog->logInfo("Transactions manager test",
//                  "_________________________________________________");

    CommandUUID command1UUID;
    BaseUserCommand *command1 = new UpdateTrustLineCommand(command1UUID, "550e8400-e29b-41d4-a716-446655440000\t200\n");

    CommandUUID command2UUID;
    BaseUserCommand *command2 = new UpdateTrustLineCommand(command2UUID, "550e8400-e29b-41d4-a716-446655440001\t200\n");

    mTransactionsManager->processCommand(BaseUserCommand::Shared(command1));
    mTransactionsManager->processCommand(BaseUserCommand::Shared(command2));
}

void TransactionsManagerTest::runAllCases() {
//    mLog->mFileLogger->writeEmptyLine(1);
//    mLog->logInfo("Transactions manager test",
//                  "_________________________________________________");
//    mLog->logInfo("Transactions manager test",
//                  "Run all previous cases step by step.");
//    mLog->logInfo("Transactions manager test",
//                  "_________________________________________________");
    runFewTransactionsWithSuccessResultTestCase();
    runSingleTransactionWithFailureStateAndAfterRelaunchingWithSuccessResultTestCase();
    runSingleTransactionWhichCrashWhileRunning();
}

void TransactionsManagerTest::run() {
    runFewTransactionsWhichReturnsFailureStateAndAfterRelaunchingWithSuccessResultTestCase();
}



