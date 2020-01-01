#include "HistoryPaymentsAllEquivalentsTransaction.h"

HistoryPaymentsAllEquivalentsTransaction::HistoryPaymentsAllEquivalentsTransaction(
    HistoryPaymentsAllEquivalentsCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::HistoryPaymentsAllEquivalentsTransactionType,
        0,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst HistoryPaymentsAllEquivalentsTransaction::run()
{
    auto ioTransaction = mStorageHandler->beginTransaction();

    if (mCommand->isPaymentRecordCommandUUIDPresent()) {
        auto const paymentRecords = ioTransaction->historyStorage()->paymentRecordsByCommandUUID(
                mCommand->paymentRecordCommandUUID());
        if (paymentRecords.size() > 1) {
            warning() << "Count transactions with requested commandUUID is more than one";
        }
        return resultOk(paymentRecords);
    }

    auto const paymentRecords = ioTransaction->historyStorage()->paymentRecordsAllEquivalents(
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

TransactionResult::SharedConst HistoryPaymentsAllEquivalentsTransaction::resultOk(
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
                    "HistoryPaymentsAllEquivalentsTransaction::resultOk: "
                    "unexpected operation type occurred.");
        }

        stream << kTokensSeparator << kRecord->equivalent()
               << kTokensSeparator << kRecord->operationUUID()
               << kTokensSeparator << kUnixTimestampMicrosec
               << kTokensSeparator << kRecord->contractor()->outputString()
               << kTokensSeparator << formattedOperationType
               << kTokensSeparator << kRecord->amount()
               << kTokensSeparator << kRecord->balanceAfterOperation()
               << kTokensSeparator << kRecord->payload();
    }

    auto result = stream.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            result));
}

const string HistoryPaymentsAllEquivalentsTransaction::logHeader() const
{
    stringstream s;
    s << "[HistoryPaymentsAllEquivalentsTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}