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

PublicKeysSharingSourceTransaction::PublicKeysSharingSourceTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :

    BaseTransaction(
        buffer,
        nodeUUID,
        logger),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore)
{
    auto bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    mStep = Stages::Recovery;

    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    auto publicKey = make_shared<lamport::PublicKey>(
        buffer.get() + bytesBufferOffset);
    mCurrentPublicKey = publicKey;
    bytesBufferOffset += mCurrentPublicKey->keySize();

    auto *keyNumber = new (buffer.get() + bytesBufferOffset) KeyNumber;
    mCurrentKeyNumber = (KeyNumber) *keyNumber;
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::run()
{
    switch (mStep) {
        case Stages::Initialisation: {
            return runInitialisationStage();
        }
        case Stages::SendNextKey: {
            return runSendNextKeyStage();
        }
        case Stages::Recovery: {
            return runRecoveryStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep");
    }
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::runInitialisationStage()
{
    info() << "runInitialisationStage with " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::KeysPending) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    try {
        keyChain.removeUnusedOwnKeys(
            ioTransaction);
        keyChain.generateKeyPairsSet(
            ioTransaction);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't generate key pairs. Details: " << e.what();
        throw e;
    }
    info() << "All keys saved";
    mCurrentKeyNumber = 0;

    mCurrentPublicKey = keyChain.publicKey(
        ioTransaction,
        mCurrentKeyNumber);
    if (mCurrentPublicKey == nullptr) {
        warning() << "There are no data for keyNumber " << mCurrentKeyNumber;
        // todo run reset keys sharing TA
        return resultDone();
    }

    try {
        auto bytesAndCount = serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        info() << "Transaction saved";
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't serialize TA. Details " << e.what();
        throw e;
    }

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
        warning() << "No confirmation message received. Transaction will be closed, and wait for message";
        return resultDone();
    }

    auto message = popNextMessage<PublicKeyHashConfirmation>();
    if (message->senderUUID != mContractorUUID) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }

    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Something wrong, because TL must be created";
        // todo : need correct reaction
        return resultDone();
    }

    processConfirmationMessage(message);
    if (message->state() != ConfirmationMessage::OK) {
        warning() << "Contractor didn't accept public key. Response code: " << message->state();
        // todo run reset keys sharing TA
        return resultDone();
    }

    if (message->number() != mCurrentKeyNumber || *message->hashConfirmation() != *mCurrentPublicKey->hash()) {
        warning() << "Number " << message->number() << " or Hash is incorrect";
        // todo run reset keys sharing TA
        return resultDone();
    }

    info() << "Key number: " << mCurrentKeyNumber << " confirmed";
    mCurrentKeyNumber++;
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    if (mCurrentKeyNumber >= TrustLineKeychain::kDefaultKeysSetSize) {
        info() << "all keys confirmed";
        auto ioTransaction = mStorageHandler->beginTransaction();
        try {
            if (mTrustLines->auditNumber(mContractorUUID) == 0) {
                if (keyChain.contractorKeysPresent(ioTransaction)) {
                    mTrustLines->setTrustLineState(
                        mContractorUUID,
                        TrustLine::AuditPending,
                        ioTransaction);
                    info() << "Waiting for initial audit";
                } else {
                    info() << "Waiting for contractor public keys";
                }
            } else {
                mTrustLines->setTrustLineState(
                    mContractorUUID,
                    TrustLine::Active,
                    ioTransaction);
                info() << "TL is ready for using";
            }
            // delete this transaction from storage
            ioTransaction->transactionHandler()->deleteRecord(
                currentTransactionUUID());
        } catch (IOError &e) {
            ioTransaction->rollback();
            error() << "Can't update TL state. Details " << e.what();
            throw e;
        }
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    mCurrentPublicKey = keyChain.publicKey(
        ioTransaction,
        mCurrentKeyNumber);
    if (mCurrentPublicKey == nullptr) {
        warning() << "There are no data for keyNumber " << mCurrentKeyNumber;
        // todo run reset keys sharing TA
        return resultDone();
    }

    try {
        auto bytesAndCount = serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        info() << "Transaction saved";
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't serialize TA. Details " << e.what();
        throw e;
    }

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

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::runRecoveryStage()
{
    info() << "Recovery";
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent " << mContractorUUID;
        return resultDone();
    }
    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::KeysPending) {
        warning() << "Invalid TL state for this TA state: "
                  << mTrustLines->trustLineState(mContractorUUID);
        return resultDone();
    }
    mStep = SendNextKey;
    return runSendNextKeyStage();
}

pair<BytesShared, size_t> PublicKeysSharingSourceTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + lamport::PublicKey::keySize()
                        + sizeof(KeyNumber);

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mContractorUUID.data,
        NodeUUID::kBytesSize);
    dataBytesOffset += NodeUUID::kBytesSize;

    // todo save only key number and read public key from storage on recovery stage
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mCurrentPublicKey->data(),
        mCurrentPublicKey->keySize());
    dataBytesOffset += mCurrentPublicKey->keySize();

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mCurrentKeyNumber,
        sizeof(KeyNumber));

    return make_pair(
        dataBytesShared,
        bytesCount);
}

const string PublicKeysSharingSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[PublicKeysSharingSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}