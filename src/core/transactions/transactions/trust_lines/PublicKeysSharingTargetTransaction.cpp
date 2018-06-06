#include "PublicKeysSharingTargetTransaction.h"

PublicKeysSharingTargetTransaction::PublicKeysSharingTargetTransaction(
    const NodeUUID &nodeUUID,
    PublicKeyMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
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
    mKeyChain(KeyChain::makeKeyChain(
        manager->trustLineReadOnly(message->senderUUID)->trustLineID(),
        logger))
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
    info() << "Receive key number: " << mMessage->number() << " key: " << (int)*mMessage->publicKey().key();
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mTrustLines->setTrustLineState(
            ioTransaction,
            mMessage->senderUUID,
            TrustLine::KeysPending);
        mKeyChain.saveContractorPublicKey(
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
    info() << "Key saved, send crc confirmation " << mMessage->publicKey().crc();
    sendMessage<PublicKeyCRCConfirmation>(
        mMessage->senderUUID,
        mMessage->equivalent(),
        mNodeUUID,
        mMessage->transactionUUID(),
        mMessage->number(),
        mMessage->publicKey().crc());
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
    info() << "Receive key number: " << message->number() << " key: " << (int)*message->publicKey().key();
    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        try {
            mKeyChain.saveContractorPublicKey(
                ioTransaction,
                message->number(),
                message->publicKey());
        } catch (IOError &e) {
            ioTransaction->rollback();
            error() << "Can't store contractor public key. Details " << e.what();
            return resultDone();
        }
    }
    info() << "Key saved, send crc confirmation " << message->publicKey().crc();
    sendMessage<PublicKeyCRCConfirmation>(
        message->senderUUID,
        message->equivalent(),
        mNodeUUID,
        message->transactionUUID(),
        message->number(),
        message->publicKey().crc());
    mReceivedKeysCount++;
    if (mReceivedKeysCount >= kKeysCount) {
        info() << "All keys received";
        auto ioTransaction = mStorageHandler->beginTransaction();
        if (mKeyChain.isAllKeysReady(ioTransaction, kKeysCount)) {
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