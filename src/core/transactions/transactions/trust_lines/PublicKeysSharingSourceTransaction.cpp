#include "PublicKeysSharingSourceTransaction.h"

PublicKeysSharingSourceTransaction::PublicKeysSharingSourceTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::PublicKeysSharingSourceTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore)
{}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::run()
{
    switch (mStep) {
        case Stages::Initialisation: {
            return runInitialisationStage();
        }
        case Stages::SendNextKey: {
            return runSendNextKeyStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep");
    }
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::runInitialisationStage()
{
    info() << "runInitialisationStage";
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    try {
        keyChain.generateKeyPairsSet(
            ioTransaction);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't generate public keys. Details: " << e.what();
        return resultDone();
    }
    info() << "All keys saved";
    mCurrentKeyNumber = 0;
    // todo check on errors
    mCurrentPublicKey = keyChain.publicKey(
        ioTransaction,
        mCurrentKeyNumber).first;
    sendMessage<PublicKeyMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        mCurrentKeyNumber,
        mCurrentPublicKey);
    info() << "Send key number: " << mCurrentKeyNumber;
    mStep = SendNextKey;
    return resultWaitForMessageTypes(
        {Message::TrustLines_HashConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::runSendNextKeyStage()
{
    if (mContext.empty()) {
        warning() << "No confirmation message received";
        return resultDone();
    }
    auto message = popNextMessage<PublicKeyHashConfirmation>();
    if (message->number() != mCurrentKeyNumber || *message->hashConfirmation() != *mCurrentPublicKey->hash()) {
        warning() << "Number or Hash is incorrect";
        return resultDone();
    }
    info() << "Key number: " << mCurrentKeyNumber << " confirmed";
    mCurrentKeyNumber++;
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    if (mCurrentKeyNumber >= TrustLineKeychain::kDefaultKeysSetSize) {
        info() << "all keys confirmed";
        auto ioTransaction = mStorageHandler->beginTransaction();
        if (keyChain.areKeysReady(ioTransaction)) {
            info() << "All Keys Ready";
            try {
                mTrustLines->setTrustLineState(
                    ioTransaction,
                    mContractorUUID,
                    TrustLine::AuditPending);
            } catch (IOError &e) {
                ioTransaction->rollback();
                error() << "Can't update TL state. Details " << e.what();
                return resultDone();
            }
            info() << "Waiting for initial audit";
        } else {
            info() << "Waiting for contractor public keys";
        }
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    // todo check on errors
    mCurrentPublicKey = keyChain.publicKey(
        ioTransaction,
        mCurrentKeyNumber).first;
    sendMessage<PublicKeyMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        mCurrentKeyNumber,
        mCurrentPublicKey);
    info() << "Send key number: " << mCurrentKeyNumber;
    return resultWaitForMessageTypes(
        {Message::TrustLines_HashConfirmation},
        kWaitMillisecondsForResponse);
}

const string PublicKeysSharingSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[PublicKeysSharingSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}