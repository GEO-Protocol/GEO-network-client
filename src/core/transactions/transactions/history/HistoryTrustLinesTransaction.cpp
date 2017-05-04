#include "HistoryTrustLinesTransaction.h"

HistoryTrustLinesTransaction::HistoryTrustLinesTransaction(
    NodeUUID &nodeUUID,
    HistoryTrustLinesCommand::Shared command,
    StorageHandler *storageHandler,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::HistoryTrustLinesTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{}

HistoryTrustLinesCommand::Shared HistoryTrustLinesTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst HistoryTrustLinesTransaction::run()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto const trustLineRecords = ioTransaction->historyStorage()->allTrustLineRecords(
        mCommand->historyCount(),
        mCommand->historyFrom());
    return resultOk(trustLineRecords);
}

TransactionResult::SharedConst HistoryTrustLinesTransaction::resultOk(
    vector<TrustLineRecord::Shared> trustLineRecords)
{
    stringstream s;
    s << trustLineRecords.size();
    for (auto const &trustLineRecord : trustLineRecords) {
        s << "\t" << trustLineRecord->operationUUID() << "\t";
        s << trustLineRecord->timestamp() << "\t";
        s << trustLineRecord->contractorUUID() << "\t";
        s << trustLineRecord->trustLineOperationType() << "\t";
        s << trustLineRecord->amount();
    }
    string historyTrustLinesStrResult = s.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            historyTrustLinesStrResult));
}

const string HistoryTrustLinesTransaction::logHeader() const
{
    stringstream s;
    s << "[HistoryTrustLinesTA]";
    return s.str();
}
