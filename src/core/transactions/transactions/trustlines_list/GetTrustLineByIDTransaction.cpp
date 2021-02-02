#include "GetTrustLineByIDTransaction.h"

GetTrustLineByIDTransaction::GetTrustLineByIDTransaction(
    GetTrustLineByIDCommand::Shared command,
    TrustLinesManager *trustLinesManager,
    Logger &logger)
    noexcept:
    BaseTransaction(
        BaseTransaction::TrustLineOneByID,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLinesManager(trustLinesManager)
{}

TransactionResult::SharedConst GetTrustLineByIDTransaction::run()
{
    if (!mTrustLinesManager->trustLineIsPresent(mCommand->contractorID())) {
        return resultTrustLineIsAbsent();
    }
    stringstream ss;
    auto kContractorTrustLine = mTrustLinesManager->trustLineReadOnly(mCommand->contractorID());
    ss << mCommand->contractorID() << kTokensSeparator;
    ss << kContractorTrustLine->state() << kTokensSeparator;
    ss << kContractorTrustLine->isOwnKeysPresent() << kTokensSeparator;
    ss << kContractorTrustLine->isContractorKeysPresent() << kTokensSeparator;
    ss << kContractorTrustLine->currentAuditNumber() << kTokensSeparator;
    ss << kContractorTrustLine->incomingTrustAmount() << kTokensSeparator;
    ss << kContractorTrustLine->outgoingTrustAmount() << kTokensSeparator;
    ss << kContractorTrustLine->balance();
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

TransactionResult::SharedConst GetTrustLineByIDTransaction::resultTrustLineIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustLineIsAbsent());
}

const string GetTrustLineByIDTransaction::logHeader() const
{
    stringstream s;
    s << "[GetTrustLineByIDTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}