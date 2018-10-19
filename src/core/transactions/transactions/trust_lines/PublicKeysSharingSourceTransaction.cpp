#include "PublicKeysSharingSourceTransaction.h"

PublicKeysSharingSourceTransaction::PublicKeysSharingSourceTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::PublicKeysSharingSourceTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mCurrentKeyNumber(0),
    mKeysCount(crypto::TrustLineKeychain::kDefaultKeysSetSize),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{
    mStep = Initialization;
}

PublicKeysSharingSourceTransaction::PublicKeysSharingSourceTransaction(
    const NodeUUID &nodeUUID,
    ShareKeysCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::PublicKeysSharingSourceTransactionType,
        nodeUUID,
        mCommand->equivalent(),
        logger),
    mCommand(command),
    mContractorUUID(mCommand->contractorUUID()),
    mCurrentKeyNumber(0),
    mKeysCount(crypto::TrustLineKeychain::kDefaultKeysSetSize),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{
    mStep = CommandInitialization;
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::run()
{
    switch (mStep) {
        case Stages::Initialization: {
            return runPublicKeysSharingInitializationStage();
        }
        case Stages::CommandInitialization: {
            return runCommandPublicKeysSharingInitializationStage();
        }
        case Stages::ResponseProcessing: {
            return runPublicKeysSendNextKeyStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep");
    }
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::runPublicKeysSharingInitializationStage()
{
    info() << "runPublicKeysSharingInitializationStage with " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Archived) {
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
        info() << "All keys saved";

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnKeysSharingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnKeysSharingStage();
#endif

        mCurrentPublicKey = keyChain.publicKey(
            ioTransaction,
            mCurrentKeyNumber);
        if (mCurrentPublicKey == nullptr) {
            warning() << "There are no data for keyNumber " << mCurrentKeyNumber;
            // todo run reset keys sharing TA
            return resultDone();
        }
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't generate public keys. Details: " << e.what();
        throw e;
    }

    sendMessage<PublicKeysSharingInitMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mKeysCount,
        mCurrentKeyNumber,
        mCurrentPublicKey);
    info() << "Send key number: " << mCurrentKeyNumber;

    mStep = ResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_HashConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::runCommandPublicKeysSharingInitializationStage()
{
    info() << "runCommandPublicKeysSharingInitializationStage with " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::Archived) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
        return resultProtocolError();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    try {
        keyChain.removeUnusedOwnKeys(
            ioTransaction);
        keyChain.generateKeyPairsSet(
            ioTransaction);
        info() << "All keys saved";

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnKeysSharingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnKeysSharingStage();
#endif

        mCurrentPublicKey = keyChain.publicKey(
            ioTransaction,
            mCurrentKeyNumber);
        if (mCurrentPublicKey == nullptr) {
            warning() << "There are no data for keyNumber " << mCurrentKeyNumber;
            // todo run reset keys sharing TA
            return resultUnexpectedError();
        }
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't generate public keys. Details: " << e.what();
        return resultUnexpectedError();
    }

    sendMessage<PublicKeysSharingInitMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mKeysCount,
        mCurrentKeyNumber,
        mCurrentPublicKey);
    info() << "Send key number: " << mCurrentKeyNumber;

    mStep = ResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::runPublicKeysSendNextKeyStage()
{
    info() << "runPublicKeysSendNextKeyStage";
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

    if (message->state() != ConfirmationMessage::OK) {
        warning() << "Contractor didn't accept public key. Response code: " << message->state();
        // todo run reset keys sharing TA
        return resultDone();
    }

    if (message->number() < mCurrentKeyNumber) {
        info() << "message key number " << message->number() << " is less than current. ignore it";
        return resultContinuePreviousState();
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
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::Active,
                ioTransaction);
            mTrustLines->setIsOwnKeysPresent(
                mContractorUUID,
                true);
            processConfirmationMessage(message);
            info() << "TL is ready for using";
        } catch (IOError &e) {
            ioTransaction->rollback();
            error() << "Can't update TL state. Details " << e.what();
            throw e;
        }
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mCurrentPublicKey = keyChain.publicKey(
            ioTransaction,
            mCurrentKeyNumber);
        if (mCurrentPublicKey == nullptr) {
            warning() << "There are no data for keyNumber " << mCurrentKeyNumber;
            // todo run reset keys sharing TA
            return resultDone();
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnKeysSharingProcessingResponseStage();
        mTrustLinesInfluenceController->testTerminateProcessOnKeysSharingProcessingResponseStage();
#endif

        processConfirmationMessage(message);
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

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_HashConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string PublicKeysSharingSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[PublicKeysSharingSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}