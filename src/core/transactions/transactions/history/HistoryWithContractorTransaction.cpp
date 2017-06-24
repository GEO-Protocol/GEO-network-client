#include "HistoryWithContractorTransaction.h"

HistoryWithContractorTransaction::HistoryWithContractorTransaction(
    NodeUUID &nodeUUID,
    HistoryWithContractorCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::HistoryWithContractorTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{}

HistoryWithContractorCommand::Shared HistoryWithContractorTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst HistoryWithContractorTransaction::run()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto const resultRecords = ioTransaction->historyStorage()->recordsWithContractor(
        mCommand->contractorUUID(),
        mCommand->historyCount(),
        mCommand->historyFrom());
    return resultOk(resultRecords);
}

TransactionResult::SharedConst HistoryWithContractorTransaction::resultOk(
    const vector<Record::Shared> &records)
{
    const auto kUnixEpoch = DateTime(boost::gregorian::date(1970,1,1));
    const auto kSeparator = BaseUserCommand::kTokensSeparator;

    stringstream stream;
    stream << records.size();
    for (auto const &kRecord : records) {
        // Formatting operation date time to the Unix timestamp
        const auto kUnixTimestampMicrosec = (kRecord->timestamp() - kUnixEpoch).total_microseconds();

        if (kRecord->isPaymentRecord()) {
            auto paymentRecord = static_pointer_cast<PaymentRecord>(kRecord);
            // Formatting operation type
            const auto kOperationType = paymentRecord->paymentOperationType();
            string formattedOperationType = "";
            if (kOperationType == PaymentRecord::IncomingPaymentType) {
                formattedOperationType = "incoming";

            } else if (kOperationType == PaymentRecord::OutgoingPaymentType) {
                formattedOperationType = "outgoing";

            } else {
                throw RuntimeError(
                        "HistoryWithContractorTransaction::resultOk: "
                                "unexpected payment operation type occured.");
            }

            stream << kSeparator << paymentRecord->recordType() << kSeparator;
            stream << paymentRecord->operationUUID() << kSeparator;
            stream << kUnixTimestampMicrosec << kSeparator;
            stream << formattedOperationType << kSeparator;
            stream << paymentRecord->amount() << kSeparator;
            stream << paymentRecord->balanceAfterOperation();
        } else if (kRecord->isTrustLineRecord()) {
            auto trustLineRecord = static_pointer_cast<TrustLineRecord>(kRecord);

            // Formatting operation type
            const auto kOperationType = trustLineRecord->trustLineOperationType();
            string formattedOperationType = "";
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

            } else {
                throw RuntimeError(
                        "HistoryWithContractorTransaction::resultOk: "
                            "unexpected trust line operation type occured.");
            }

            stream << kSeparator << trustLineRecord->recordType() << kSeparator;
            stream << trustLineRecord->operationUUID() << kSeparator;
            stream << kUnixTimestampMicrosec << kSeparator;
            stream << formattedOperationType << kSeparator;
            stream << trustLineRecord->amount();
        } else {
            throw ValueError("HistoryWithContractorTransaction::resultOk: "
                                 "unexpected record type occured.");
        }
    }

    auto result = stream.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            result));
}

const string HistoryWithContractorTransaction::logHeader() const
{
    stringstream s;
    s << "[HistoryWithContractorTA: " << currentTransactionUUID() << "]";
    return s.str();
}
