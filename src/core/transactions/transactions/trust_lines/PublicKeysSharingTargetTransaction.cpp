#include "PublicKeysSharingTargetTransaction.h"

PublicKeysSharingTargetTransaction::PublicKeysSharingTargetTransaction(
    const NodeUUID &nodeUUID,
    PublicKeyMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::PublicKeysSharingTargetTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore)
{}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::run()
{
    info() << "Receive key number: " << mMessage->number() << " from " << mMessage->senderUUID;

    if (!mTrustLines->trustLineIsPresent(mMessage->senderUUID)) {
        sendMessage<PublicKeyHashConfirmation>(
            mMessage->senderUUID,
            mMessage->equivalent(),
            mNodeUUID,
            mMessage->transactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        warning() << "Trust line is absent.";
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mMessage->senderUUID));
    try {
        if (mMessage->number() == 0) {
            info() << "init key number";
            if (mTrustLines->auditNumber(mMessage->senderUUID) == 0) {
                if (mTrustLines->trustLineState(mMessage->senderUUID) != TrustLine::Init
                    and mTrustLines->trustLineState(mMessage->senderUUID) != TrustLine::KeysPending) {
                    warning() << "invalid TL state " << mTrustLines->trustLineState(mMessage->senderUUID)
                              << " for init TL. Waiting for state updating";
                    return resultDone();
                }
            } else {
                if (mTrustLines->trustLineState(mMessage->senderUUID) != TrustLine::Active) {
                    warning() << "invalid TL state " << mTrustLines->trustLineState(mMessage->senderUUID)
                              << ". Waiting for state updating";
                    return resultDone();
                }
            }
            keyChain.removeUnusedContractorKeys(ioTransaction);
            mTrustLines->setTrustLineState(
                mMessage->senderUUID,
                TrustLine::KeysPending,
                ioTransaction);
        } else {
            if (mTrustLines->trustLineState(mMessage->senderUUID) != TrustLine::KeysPending) {
                sendMessage<PublicKeyHashConfirmation>(
                    mMessage->senderUUID,
                    mMessage->equivalent(),
                    mNodeUUID,
                    mMessage->transactionUUID(),
                    ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
                warning() << "invalid TL state " << mTrustLines->trustLineState(mMessage->senderUUID);
                return resultDone();
            }
        }
        keyChain.setContractorPublicKey(
            ioTransaction,
            mMessage->number(),
            mMessage->publicKey());
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't store contractor public key. Details " << e.what();
        sendMessage<PublicKeyHashConfirmation>(
            mMessage->senderUUID,
            mMessage->equivalent(),
            mNodeUUID,
            mMessage->transactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        throw e;
    }
    info() << "Key saved, send hash confirmation";
    sendMessage<PublicKeyHashConfirmation>(
        mMessage->senderUUID,
        mMessage->equivalent(),
        mNodeUUID,
        mMessage->transactionUUID(),
        mMessage->number(),
        mMessage->publicKey()->hash());

    if (keyChain.allContractorKeysPresent(ioTransaction)) {
        info() << "All keys received";
        // todo check if count of keys are equal on both sides: source and target
        // maybe add new message
        try {
            if (mTrustLines->auditNumber(mMessage->senderUUID) == 0) {
                if (keyChain.ownKeysPresent(ioTransaction)) {
                    mTrustLines->setTrustLineState(
                        mMessage->senderUUID,
                        TrustLine::AuditPending,
                        ioTransaction);
                    info() << "Start initial audit";
                    const auto transaction = make_shared<InitialAuditSourceTransaction>(
                        mNodeUUID,
                        mMessage->senderUUID,
                        mEquivalent,
                        mTrustLines,
                        mStorageHandler,
                        mKeysStore,
                        mLog);
                    launchSubsidiaryTransaction(transaction);
                } else {
                    info() << "Start sharing own keys";
                    const auto transaction = make_shared<PublicKeysSharingSourceTransaction>(
                        mNodeUUID,
                        mMessage->senderUUID,
                        mEquivalent,
                        mTrustLines,
                        mStorageHandler,
                        mKeysStore,
                        mLog);
                    launchSubsidiaryTransaction(transaction);
                }
            } else {
                mTrustLines->setTrustLineState(
                    mMessage->senderUUID,
                    TrustLine::Active,
                    ioTransaction);
                info() << "TL is ready for using";
            }
        } catch (IOError &e) {
            ioTransaction->rollback();
            error() << "Can't update TL state. Details " << e.what();
            throw e;
        }
    }
    return resultDone();
}

const string PublicKeysSharingTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[PublicKeySharingTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}