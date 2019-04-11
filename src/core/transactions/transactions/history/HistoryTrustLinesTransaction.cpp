#include "HistoryTrustLinesTransaction.h"

HistoryTrustLinesTransaction::HistoryTrustLinesTransaction(
    HistoryTrustLinesCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::HistoryTrustLinesTransactionType,
        command->equivalent(),
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst HistoryTrustLinesTransaction::run()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto const trustLineRecords = ioTransaction->historyStorage()->allTrustLineRecords(
        mCommand->equivalent(),
        mCommand->historyCount(),
        mCommand->historyFrom(),
        mCommand->timeFrom(),
        mCommand->isTimeFromPresent(),
        mCommand->timeTo(),
        mCommand->isTimeToPresent());
    return resultOk(trustLineRecords);
}

TransactionResult::SharedConst HistoryTrustLinesTransaction::resultOk(
    const vector<TrustLineRecord::Shared> &records)
{
    const auto kUnixEpoch = DateTime(boost::gregorian::date(1970,1,1));

    stringstream stream;
    stream << records.size();
    for (auto const &kRecord : records) {
        // Formatting operation date time to the Unix timestamp
        const auto kUnixTimestampMicrosec = (kRecord->timestamp() - kUnixEpoch).total_microseconds();

        // Formatting operation type
        const auto kOperationType = kRecord->trustLineOperationType();
        string formattedOperationType;
        if (kOperationType == TrustLineRecord::Opening) {
            formattedOperationType = "opening";

        } else if (kOperationType == TrustLineRecord::Accepting) {
            formattedOperationType = "accepting";

        } else if (kOperationType == TrustLineRecord::Setting) {
            formattedOperationType = "setting";

        } else if (kOperationType == TrustLineRecord::Updating) {
            formattedOperationType = "updating";

        } else if (kOperationType == TrustLineRecord::Closing) {
            formattedOperationType = "closing";

        } else if (kOperationType == TrustLineRecord::Rejecting) {
            formattedOperationType = "rejecting";

        } else if (kOperationType == TrustLineRecord::ClosingIncoming) {
            formattedOperationType = "closing_incoming";

        } else if (kOperationType == TrustLineRecord::RejectingOutgoing) {
            formattedOperationType = "rejecting_outgoing";

        } else {
            throw RuntimeError(
                "HistoryTrustLinesTransaction::resultOk: "
                "unexpected operation type occurred.");
        }

        stream << kTokensSeparator << kRecord->operationUUID()
               << kTokensSeparator << kUnixTimestampMicrosec
               << kTokensSeparator << kRecord->contractor()->outputString()
               << kTokensSeparator << formattedOperationType
               << kTokensSeparator << kRecord->amount();
    }

    auto result = stream.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            result));
}

const string HistoryTrustLinesTransaction::logHeader() const
{
    stringstream s;
    s << "[HistoryTrustLinesTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
