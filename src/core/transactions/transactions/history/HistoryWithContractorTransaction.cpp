#include "HistoryWithContractorTransaction.h"

HistoryWithContractorTransaction::HistoryWithContractorTransaction(
    HistoryWithContractorCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::HistoryWithContractorTransactionType,
        command->equivalent(),
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst HistoryWithContractorTransaction::run()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto contractorAddresses = mCommand->contractorAddresses();
    auto const resultRecords = ioTransaction->historyStorage()->recordsWithContractor(
        contractorAddresses,
        mCommand->equivalent(),
        mCommand->historyCount(),
        mCommand->historyFrom());
    return resultOk(resultRecords);
}

TransactionResult::SharedConst HistoryWithContractorTransaction::resultOk(
    const vector<Record::Shared> &records)
{
    const auto kUnixEpoch = DateTime(boost::gregorian::date(1970,1,1));

    stringstream stream;
    stream << records.size();
    for (auto const &kRecord : records) {
        // Formatting operation date time to the Unix timestamp
        const auto kUnixTimestampMicrosec = (kRecord->timestamp() - kUnixEpoch).total_microseconds();

        string formattedRecordType;
        if (kRecord->isPaymentRecord()) {
            formattedRecordType = "payment";
            auto paymentRecord = static_pointer_cast<PaymentRecord>(kRecord);
            // Formatting operation type
            const auto kOperationType = paymentRecord->paymentOperationType();
            string formattedOperationType;
            if (kOperationType == PaymentRecord::IncomingPaymentType) {
                formattedOperationType = "incoming";

            } else if (kOperationType == PaymentRecord::OutgoingPaymentType) {
                formattedOperationType = "outgoing";

            } else {
                throw RuntimeError(
                        "HistoryWithContractorTransaction::resultOk: "
                                "unexpected payment operation type occurred.");
            }

            stream << kTokensSeparator << formattedRecordType
                   << kTokensSeparator << paymentRecord->operationUUID()
                   << kTokensSeparator << kUnixTimestampMicrosec
                   << kTokensSeparator << formattedOperationType
                   << kTokensSeparator << paymentRecord->amount()
                   << kTokensSeparator << paymentRecord->balanceAfterOperation();

        } else if (kRecord->isTrustLineRecord()) {
            formattedRecordType = "trustline";
            auto trustLineRecord = static_pointer_cast<TrustLineRecord>(kRecord);

            // Formatting operation type
            const auto kOperationType = trustLineRecord->trustLineOperationType();
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
                        "HistoryWithContractorTransaction::resultOk: "
                            "unexpected trust line operation type occurred.");
            }

            stream << kTokensSeparator << formattedRecordType
                   << kTokensSeparator << trustLineRecord->operationUUID()
                   << kTokensSeparator << kUnixTimestampMicrosec
                   << kTokensSeparator << formattedOperationType
                   << kTokensSeparator << trustLineRecord->amount();
        } else {
            throw ValueError("HistoryWithContractorTransaction::resultOk: "
                                 "unexpected record type occurred.");
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
    s << "[HistoryWithContractorTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
