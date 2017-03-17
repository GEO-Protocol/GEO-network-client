#include "HistoryTrustLinesTransaction.h"

HistoryTrustLinesTransaction::HistoryTrustLinesTransaction(
    NodeUUID &nodeUUID,
    HistoryTrustLinesCommand::Shared command,
    OperationsHistoryStorage *historyStorage,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::HistoryTrustLinesTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mHistoryStorage(historyStorage) {}

HistoryTrustLinesCommand::Shared HistoryTrustLinesTransaction::command() const {

    return mCommand;
}

TransactionResult::SharedConst HistoryTrustLinesTransaction::run() {

    info() << "run\t";
    info() << "run\t" << "from: " << mCommand->historyFrom();
    info() << "run\t" << "count: " << mCommand->historyCount();

    vector<TrustLineRecord::Shared> trustLineRecords = mHistoryStorage->trustLineRecordsStack(
        mCommand->historyCount(),
        mCommand->historyFrom());

    info() << "run\t" << "real count: " << trustLineRecords.size();

    return resultOk(trustLineRecords);

}

TransactionResult::SharedConst HistoryTrustLinesTransaction::resultOk(
        vector<TrustLineRecord::Shared> trustLineRecords) {

    stringstream s;
    s << trustLineRecords.size();
    for (auto &trustLineRecord : trustLineRecords) {
        s << "\t" << trustLineRecord->operationUUID() << "\t";
        s << trustLineRecord->operationTimestamp() << "\t";
        s << trustLineRecord->contractorUUID() << "\t";
        s << trustLineRecord->trustLineOperationType() << "\t";
        s << trustLineRecord->amount();
    }
    string historyTrustLinesStrResult = s.str();
    return transactionResultFromCommand(mCommand->resultOk(historyTrustLinesStrResult));
}

const string HistoryTrustLinesTransaction::logHeader() const
{
    stringstream s;
    s << "[HistoryTrustLinesTA]";

    return s.str();
}
