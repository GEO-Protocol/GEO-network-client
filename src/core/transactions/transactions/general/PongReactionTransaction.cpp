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
    mContractorID(message->idOnReceiverSide),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst PongReactionTransaction::run()
{
    info() << "sender " << mContractorID;
    bool isSignalSend = false;
    for (const auto equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        auto trustLineManager = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent);

        if (!trustLineManager->trustLineIsPresent(mContractorID)) {
            continue;
        }

        if (trustLineManager->trustLineState(mContractorID) == TrustLine::Init) {
            mResumeTransactionSignal(
                mContractorUUID,
                mContractorID,
                equivalent,
                BaseTransaction::OpenTrustLineTransaction);
            isSignalSend = true;
        } else if (trustLineManager->trustLineState(mContractorID) == TrustLine::AuditPending) {
            mResumeTransactionSignal(
                mContractorUUID,
                mContractorID,
                equivalent,
                BaseTransaction::AuditSourceTransactionType);
            isSignalSend = true;
        }
    }
    if (!isSignalSend) {
        info() << "No one signal was sent";
        processPongMessage(mContractorID);
    }
    return resultDone();
}

const string PongReactionTransaction::logHeader() const
{
    stringstream s;
    s << "[PongReactionTA: " << currentTransactionUUID() << "]";
    return s.str();
}