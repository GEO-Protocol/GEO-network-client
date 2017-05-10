#include "HistoryPaymentsTransaction.h"

HistoryPaymentsTransaction::HistoryPaymentsTransaction(
    NodeUUID &nodeUUID,
    HistoryPaymentsCommand::Shared command,
    StorageHandler *storageHandler,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::HistoryPaymentsTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{}

HistoryPaymentsCommand::Shared HistoryPaymentsTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst HistoryPaymentsTransaction::run()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto const paymentRecords = ioTransaction->historyStorage()->allPaymentRecords(
        mCommand->historyCount(),
        mCommand->historyFrom());
    return resultOk(paymentRecords);
}

TransactionResult::SharedConst HistoryPaymentsTransaction::resultOk(
    vector<PaymentRecord::Shared> paymentRecords)
{
    stringstream s;
    s << paymentRecords.size();
    for (auto const &paymentRecord : paymentRecords) {
        s << "\t" << paymentRecord->operationUUID() << "\t";
        s << paymentRecord->timestamp() << "\t";
        s << paymentRecord->contractorUUID() << "\t";
        s << paymentRecord->paymentOperationType() << "\t";
        s << paymentRecord->amount() << "\t";
        s << paymentRecord->balanceAfterOperation();
    }
    string historyPaymentsStrResult = s.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            historyPaymentsStrResult));
}

const string HistoryPaymentsTransaction::logHeader() const
{
    stringstream s;
    s << "[HistoryPaymentsTA: " << currentTransactionUUID() << "]";
    return s.str();
}
