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
    switch (mStep) {
        case Stages::Initialisation: {
            return runInitialisationStage();
        }
        case Stages::ReceiveNextKey: {
            return runReceiveNextKeyStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep");
    }
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::runInitialisationStage()
{
    info() << "Receive key number: " << mMessage->number();
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mMessage->senderUUID));
    try {
        mTrustLines->setTrustLineState(
            ioTransaction,
            mMessage->senderUUID,
            TrustLine::KeysPending);
        keyChain.setContractorPublicKey(
            ioTransaction,
            mMessage->number(),
            mMessage->publicKey());
    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        warning() << "There are no TL with " << mMessage->senderUUID;
        return resultDone();
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't update TL state or store contractor public key. Details " << e.what();
        return resultDone();
    }
    info() << "Key saved, send hash confirmation";
    sendMessage<PublicKeyHashConfirmation>(
        mMessage->senderUUID,
        mMessage->equivalent(),
        mNodeUUID,
        mMessage->transactionUUID(),
        mMessage->number(),
        mMessage->publicKey()->hash());
    mReceivedKeysCount = 1;

    mStep = ReceiveNextKey;
    return resultWaitForMessageTypes(
        {Message::TrustLines_PublicKey},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::runReceiveNextKeyStage()
{
    if (mContext.empty()) {
        warning() << "No all keys received";
        return resultDone();
    }
    auto message = popNextMessage<PublicKeyMessage>();
    if (message->senderUUID != mMessage->senderUUID) {
        warning() << "Receive message from different sender: " << message->senderUUID;
        return resultDone();
    }
    info() << "Receive key number: " << message->number();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(message->senderUUID));
    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        try {
            keyChain.setContractorPublicKey(
                ioTransaction,
                message->number(),
                message->publicKey());
        } catch (IOError &e) {
            ioTransaction->rollback();
            error() << "Can't store contractor public key. Details " << e.what();
            return resultDone();
        }
    }
    info() << "Key saved, send hash confirmation";
    sendMessage<PublicKeyHashConfirmation>(
        message->senderUUID,
        message->equivalent(),
        mNodeUUID,
        message->transactionUUID(),
        message->number(),
        message->publicKey()->hash());
    mReceivedKeysCount++;
    if (mReceivedKeysCount >= TrustLineKeychain::kDefaultKeysSetSize) {
        info() << "All keys received";
        auto ioTransaction = mStorageHandler->beginTransaction();
        if (keyChain.areKeysReady(ioTransaction)) {
            info() << "All Keys Ready";
            try {
                mTrustLines->setTrustLineState(
                    ioTransaction,
                    mMessage->senderUUID,
                    TrustLine::AuditPending);
            } catch (IOError &e) {
                ioTransaction->rollback();
                error() << "Can't update TL state. Details " << e.what();
                return resultDone();
            }
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
        return resultDone();
    }
    return resultWaitForMessageTypes(
        {Message::TrustLines_PublicKey},
        kWaitMillisecondsForResponse);
}

const string PublicKeysSharingTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[PublicKeySharingTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}