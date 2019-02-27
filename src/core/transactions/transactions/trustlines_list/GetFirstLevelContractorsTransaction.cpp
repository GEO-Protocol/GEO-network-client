#include "GetFirstLevelContractorsTransaction.h"

GetFirstLevelContractorsTransaction::GetFirstLevelContractorsTransaction(
    GetFirstLevelContractorsCommand::Shared command,
    TrustLinesManager *trustLinesManager,
    ContractorsManager *contractorsManager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::ContractorsList,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLinesManager(trustLinesManager),
    mContractorsManager(contractorsManager)
{}

TransactionResult::SharedConst GetFirstLevelContractorsTransaction::run()
{
    if (mContractorsManager != nullptr) {
        const auto contractorsCount = mContractorsManager->allContractors().size();
        stringstream ss;
        ss << to_string(contractorsCount);
        for (const auto &contractor: mContractorsManager->allContractors()) {
            ss << kTokensSeparator << contractor->getID()
               << kTokensSeparator << contractor->outputString();
        }
        ss << kCommandsSeparator;
        string kResultInfo = ss.str();
        return transactionResultFromCommand(
            mCommand->resultOk(kResultInfo));
    }
    if (mTrustLinesManager == nullptr) {
        warning() << "Both TrustLinesManager and ContractorsManager are null";
        return transactionResultFromCommand(
            mCommand->responseUnexpectedError());
    }
    const auto neighborsCount = mTrustLinesManager->trustLines().size();
    stringstream ss;
    ss << to_string(neighborsCount);
    for (const auto &nodeIDAndTrustLine: mTrustLinesManager->trustLines()) {
        ss << kTokensSeparator << nodeIDAndTrustLine.first
           << kTokensSeparator << mContractorsManager->contractor(nodeIDAndTrustLine.first)->outputString();
    }
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(kResultInfo));
}

const string GetFirstLevelContractorsTransaction::logHeader() const
{
    stringstream s;
    s << "[GetFirstLevelContractorsTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
