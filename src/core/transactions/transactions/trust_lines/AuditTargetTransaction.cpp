#include "AuditTargetTransaction.h"

AuditTargetTransaction::AuditTargetTransaction(
    const NodeUUID &nodeUUID,
    AuditMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger):
    BaseTrustLineTransaction(
        BaseTransaction::AuditTargetTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        manager,
        storageHandler,
        keystore,
        logger)
{
    mContractorUUID = message->senderUUID;
    mAuditNumber = mTrustLines->auditNumber(message->senderUUID) + 1;
    mAuditMessage = message;
}

TransactionResult::SharedConst AuditTargetTransaction::run()
{
    return runAuditTargetStage();
}

TransactionResult::SharedConst AuditTargetTransaction::sendErrorConfirmation(
    ConfirmationMessage::OperationState errorState)
{
    sendMessage<AuditMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        errorState);
    return resultDone();
}

const string AuditTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[AuditTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}