#include "GetEquivalentListTransaction.h"

GetEquivalentListTransaction::GetEquivalentListTransaction(
    NodeUUID &nodeUUID,
    EquivalentListCommand::Shared command,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    Logger &logger)
    noexcept :
    BaseTransaction(
        BaseTransaction::EquivalentsList,
        nodeUUID,
        0,
        logger),
    mCommand(command),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter)
{}

TransactionResult::SharedConst GetEquivalentListTransaction::run()
{
    const auto kEquivalentsCount = mEquivalentsSubsystemsRouter->equivalents().size();
    stringstream ss;
    ss << to_string(kEquivalentsCount);
    for (const auto &equivalent: mEquivalentsSubsystemsRouter->equivalents()) {
        ss << kTokensSeparator;
        ss << equivalent;
    }
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

const string GetEquivalentListTransaction::logHeader() const
{
    stringstream s;
    s << "[GetEquivalentListTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}