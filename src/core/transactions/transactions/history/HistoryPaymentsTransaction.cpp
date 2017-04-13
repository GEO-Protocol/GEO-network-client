#include "HistoryPaymentsTransaction.h"

HistoryPaymentsTransaction::HistoryPaymentsTransaction(
        NodeUUID &nodeUUID,
        HistoryPaymentsCommand::Shared command,
        OperationsHistoryStorage *historyStorage,
        Logger *logger) :

        BaseTransaction(
                BaseTransaction::TransactionType::HistoryPaymentsTransactionType,
                nodeUUID,
                logger),
        mCommand(command),
        mHistoryStorage(historyStorage) {}

HistoryPaymentsCommand::Shared HistoryPaymentsTransaction::command() const {

    return mCommand;
}

TransactionResult::SharedConst HistoryPaymentsTransaction::run() {

    info() << "run\t";
    info() << "run\t" << "from: " << mCommand->historyFrom();
    info() << "run\t" << "count: " << mCommand->historyCount();

    vector<PaymentRecord::Shared> paymentRecords = mHistoryStorage->paymentRecordsStack(
        mCommand->historyCount(),
        mCommand->historyFrom());

    info() << "run\t" << "real count: " << paymentRecords.size();

    return resultOk(paymentRecords);

}

TransactionResult::SharedConst HistoryPaymentsTransaction::resultOk(
        vector<PaymentRecord::Shared> paymentRecords) {

    stringstream s;
    s << paymentRecords.size();
    for (auto &paymentRecord : paymentRecords) {
        s << "\t" << paymentRecord->operationUUID() << "\t";
        s << paymentRecord->operationTimestamp() << "\t";
        s << paymentRecord->contractorUUID() << "\t";
        s << paymentRecord->paymentOperationType() << "\t";
        s << paymentRecord->amount() << "\t";
        s << paymentRecord->balanceAfterOperation();
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
