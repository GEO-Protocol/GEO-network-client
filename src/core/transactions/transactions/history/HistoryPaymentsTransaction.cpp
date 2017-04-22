#include "HistoryPaymentsTransaction.h"

HistoryPaymentsTransaction::HistoryPaymentsTransaction(
    NodeUUID &nodeUUID,
    HistoryPaymentsCommand::Shared command,
    HistoryStorage *historyStorage,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::HistoryPaymentsTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mHistoryStorage(historyStorage)
{}

HistoryPaymentsCommand::Shared HistoryPaymentsTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst HistoryPaymentsTransaction::run()
{
    auto const paymentRecords = mHistoryStorage->allPaymentRecords(
        mCommand->historyCount(),
        mCommand->historyFrom());
    return resultOk(paymentRecords);
}

TransactionResult::SharedConst HistoryPaymentsTransaction::resultOk(
    vector<pair<PaymentRecord::Shared, DateTime>> paymentRecords)
{
    stringstream s;
    s << paymentRecords.size();
    for (auto const &paymentRecordAndTimestamp : paymentRecords) {
        s << "\t" << paymentRecordAndTimestamp.first->operationUUID() << "\t";
        s << paymentRecordAndTimestamp.second << "\t";
        s << paymentRecordAndTimestamp.first->contractorUUID() << "\t";
        s << paymentRecordAndTimestamp.first->paymentOperationType() << "\t";
        s << paymentRecordAndTimestamp.first->amount() << "\t";
        s << paymentRecordAndTimestamp.first->balanceAfterOperation();
    }
    string historyPaymentsStrResult = s.str();
    return transactionResultFromCommand(mCommand->resultOk(historyPaymentsStrResult));
}

const string HistoryPaymentsTransaction::logHeader() const
{
    stringstream s;
    s << "[HistoryPaymentsTA]";
    return s.str();
}
