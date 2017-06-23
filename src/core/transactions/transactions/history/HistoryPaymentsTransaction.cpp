#include "HistoryPaymentsTransaction.h"

HistoryPaymentsTransaction::HistoryPaymentsTransaction(
    NodeUUID &nodeUUID,
    HistoryPaymentsCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger) :

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
        mCommand->historyFrom(),
        mCommand->timeFrom(),
        mCommand->isTimeFromPresent(),
        mCommand->timeTo(),
        mCommand->isTimeToPresent(),
        mCommand->lowBoundaryAmount(),
        mCommand->isLowBoundaryAmountPresent(),
        mCommand->highBoundaryAmount(),
        mCommand->isHighBoundaryAmountPresent());

    // TODO remove after testing
    debug() << "Additional payment records:";
    for (auto const &paymentRecord : ioTransaction->historyStorage()->allPaymentAdditionalRecords()) {
        debug() << paymentRecord->operationUUID() << " "
                << paymentRecord->paymentOperationType() << " "
                << paymentRecord->timestamp() << " "
                << paymentRecord->amount() << " "
                << paymentRecord->balanceAfterOperation();
    }

    return resultOk(paymentRecords);
}

TransactionResult::SharedConst HistoryPaymentsTransaction::resultOk(
    const vector<PaymentRecord::Shared> &records)
{
    const auto kUnixEpoch = DateTime(boost::gregorian::date(1970,1,1));
    const auto kSeparator = BaseUserCommand::kTokensSeparator;

    stringstream stream;
    stream << records.size();
    for (auto const &kRecord : records) {
        // Formatting operation date time to the Unix timestamp
        const auto kUnixTimestampMicrosec = (kRecord->timestamp() - kUnixEpoch).total_microseconds();

        // Formatting operation type
        const auto kOperationType = kRecord->paymentOperationType();
        string formattedOperationType = "";
        if (kOperationType == PaymentRecord::IncomingPaymentType) {
            formattedOperationType = "incoming";

        } else if (kOperationType == PaymentRecord::OutgoingPaymentType) {
            formattedOperationType = "outgoing";

        } else {
            throw RuntimeError(
                "HistoryPaymentsTransaction::resultOk: "
                "unexpected operation type occured.");
        }

        stream << kSeparator << kRecord->operationUUID() << kSeparator;
        stream << kUnixTimestampMicrosec << kSeparator;
        stream << kRecord->contractorUUID() << kSeparator;
        stream << formattedOperationType << kSeparator;
        stream << kRecord->amount() << kSeparator;
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
    s << "[HistoryPaymentsTA: " << currentTransactionUUID() << "]";
    return s.str();
}
