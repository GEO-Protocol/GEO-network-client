#include "HistoryAdditionalPaymentsTransaction.h"

HistoryAdditionalPaymentsTransaction::HistoryAdditionalPaymentsTransaction(
    NodeUUID &nodeUUID,
    HistoryAdditionalPaymentsCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::HistoryAdditionalPaymentsTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{}

HistoryAdditionalPaymentsCommand::Shared HistoryAdditionalPaymentsTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst HistoryAdditionalPaymentsTransaction::run()
{
    auto ioTransaction = mStorageHandler->beginTransaction();

    auto const paymentRecords = ioTransaction->historyStorage()->allPaymentAdditionalRecords(
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

TransactionResult::SharedConst HistoryAdditionalPaymentsTransaction::resultOk(
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
        if (kOperationType == PaymentRecord::CycleCloserType) {
            formattedOperationType = "cycle_closer";

        } else if (kOperationType == PaymentRecord::CyclerCloserIntermediateType) {
            formattedOperationType = "cycle_intermediate";

        } else if (kOperationType == PaymentRecord::IncomingPaymentType) {
            formattedOperationType = "intermediate";

        } else {
            throw RuntimeError(
                "HistoryAdditionalPaymentsTransaction::resultOk: "
                    "unexpected operation type occured.");
        }

        stream << kSeparator << kRecord->operationUUID() << kSeparator;
        stream << kUnixTimestampMicrosec << kSeparator;
        stream << formattedOperationType << kSeparator;
        stream << kRecord->amount() << kSeparator;
    }

    auto result = stream.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            result));
}

const string HistoryAdditionalPaymentsTransaction::logHeader() const
{
    stringstream s;
    s << "[HistoryAdditionalPaymentsTA: " << currentTransactionUUID() << "]";
    return s.str();
}
