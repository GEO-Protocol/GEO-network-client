#include "HistoryPaymentsTransaction.h"

HistoryPaymentsTransaction::HistoryPaymentsTransaction(
    NodeUUID &nodeUUID,
    HistoryPaymentsCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::HistoryPaymentsTransactionType,
        nodeUUID,
        command->equivalent(),
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

    if (mCommand->isPaymentRecordCommandUUIDPresent()) {
        auto const paymentRecords = ioTransaction->historyStorage()->paymentRecordsByCommandUUID(
            mCommand->paymentRecordCommandUUID());
        if (paymentRecords.size() > 1) {
            warning() << "Count transactions with given commandUUID is more than one";
        }
        return resultOk(paymentRecords);
    }

    auto const paymentRecords = ioTransaction->historyStorage()->allPaymentRecords(
        mCommand->equivalent(),
        mCommand->historyCount(),
        mCommand->historyFrom(),
        mCommand->timeFrom(),
        mCommand->isTimeFromPresent(),
        mCommand->timeTo(),
        mCommand->isTimeToPresent(),
        mCommand->lowBoundaryAmount(),
        mCommand->isLowBoundaryAmountPresent(),
        mCommand->highBoundaryAmount(),
        mCommand->isHighBoundaryAmountPresent());
    return resultOk(paymentRecords);
}

TransactionResult::SharedConst HistoryPaymentsTransaction::resultOk(
    const vector<PaymentRecord::Shared> &records)
{
    const auto kUnixEpoch = DateTime(boost::gregorian::date(1970,1,1));

    stringstream stream;
    stream << records.size();
    for (auto const &kRecord : records) {
        // Formatting operation date time to the Unix timestamp
        const auto kUnixTimestampMicrosec = (kRecord->timestamp() - kUnixEpoch).total_microseconds();

        // Formatting operation type
        const auto kOperationType = kRecord->paymentOperationType();
        string formattedOperationType;
        if (kOperationType == PaymentRecord::IncomingPaymentType) {
            formattedOperationType = "incoming";

        } else if (kOperationType == PaymentRecord::OutgoingPaymentType) {
            formattedOperationType = "outgoing";

        } else {
            throw RuntimeError(
                "HistoryPaymentsTransaction::resultOk: "
                "unexpected operation type occurred.");
        }

        stream << kTokensSeparator << kRecord->operationUUID() << kTokensSeparator;
        stream << kUnixTimestampMicrosec << kTokensSeparator;
        stream << kRecord->contractorUUID() << kTokensSeparator;
        stream << formattedOperationType << kTokensSeparator;
        stream << kRecord->amount() << kTokensSeparator;
        stream << kRecord->balanceAfterOperation();
    }

    auto result = stream.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            result));
}

const string HistoryPaymentsTransaction::logHeader() const
{
    stringstream s;
    s << "[HistoryPaymentsTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
