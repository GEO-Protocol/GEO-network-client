#include "HistoryTrustLinesTransaction.h"

HistoryTrustLinesTransaction::HistoryTrustLinesTransaction(
    NodeUUID &nodeUUID,
    HistoryTrustLinesCommand::Shared command,
    HistoryStorage *historyStorage,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::HistoryTrustLinesTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mHistoryStorage(historyStorage)
{}

HistoryTrustLinesCommand::Shared HistoryTrustLinesTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst HistoryTrustLinesTransaction::run()
{
    auto const trustLineRecords = mHistoryStorage->allTrustLineRecords(
        mCommand->historyCount(),
        mCommand->historyFrom());
    return resultOk(trustLineRecords);
}

TransactionResult::SharedConst HistoryTrustLinesTransaction::resultOk(
    vector<pair<TrustLineRecord::Shared, DateTime>> trustLineRecords)
{
    stringstream s;
    s << trustLineRecords.size();
    for (auto const &trustLineRecordAndTimestamp : trustLineRecords) {
        s << "\t" << trustLineRecordAndTimestamp.first->operationUUID() << "\t";
        s << trustLineRecordAndTimestamp.second << "\t";
        s << trustLineRecordAndTimestamp.first->contractorUUID() << "\t";
        s << trustLineRecordAndTimestamp.first->trustLineOperationType() << "\t";
        s << trustLineRecordAndTimestamp.first->amount();
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
