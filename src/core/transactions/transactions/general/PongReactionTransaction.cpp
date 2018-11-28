#include "PongReactionTransaction.h"

PongReactionTransaction::PongReactionTransaction(
    const NodeUUID &nodeUUID,
    PongMessage::Shared message,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    StorageHandler *storageHandler,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::PongReactionType,
        nodeUUID,
        0,
        logger),
    mContractorUUID(message->senderUUID),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst PongReactionTransaction::run()
{
    bool isSignalSend = false;
    for (const auto equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        auto trustLineManager = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent);

        if (!trustLineManager->trustLineIsPresent(mContractorUUID)) {
            continue;
        }

        if (trustLineManager->trustLineState(mContractorUUID) == TrustLine::Init) {
            mResumeTransactionSignal(
                mContractorUUID,
                trustLineManager->contractorID(mContractorUUID),
                equivalent,
                BaseTransaction::OpenTrustLineTransaction);
            isSignalSend = true;
        } else if (trustLineManager->trustLineState(mContractorUUID) == TrustLine::AuditPending) {
            mResumeTransactionSignal(
                mContractorUUID,
                trustLineManager->contractorID(mContractorUUID),
                equivalent,
                BaseTransaction::AuditSourceTransactionType);
            isSignalSend = true;
        }
    }
    if (!isSignalSend) {
        info() << "No one signal was sent";
        // todo : get real contractorID of sender node and process pong
        //processPongMessage(mContractorUUID);
    }
    return resultDone();
}

const string PongReactionTransaction::logHeader() const
{
    stringstream s;
    s << "[PongReactionTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}