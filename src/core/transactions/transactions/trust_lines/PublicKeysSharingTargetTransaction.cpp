#include "PublicKeysSharingTargetTransaction.h"

PublicKeysSharingTargetTransaction::PublicKeysSharingTargetTransaction(
    const NodeUUID &nodeUUID,
    PublicKeysSharingInitMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::PublicKeysSharingTargetTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mContractorID(message->idOnReceiverSide),
    mSenderIncomingIP(message->senderIncomingIP()),
    mContractorAddresses(message->senderAddresses),
    mContractorsManager(contractorsManager),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{
    mContractorKeysCount = message->keysCount();
    mCurrentKeyNumber = message->number();
    mCurrentPublicKey = message->publicKey();
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::run()
{
    switch (mStep) {
        case Stages::Initialization: {
            return runPublicKeyReceiverInitStage();
        }
        case Stages::NextKeyProcessing: {
            return runReceiveNextKeyStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                "wrong value of mStep " + to_string(mStep));
    }
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::runPublicKeyReceiverInitStage()
{
    info() << "runPublicKeyReceiverInitStage " << mContractorID << " sender incoming IP " << mSenderIncomingIP;

    if (!mContractorsManager->contractorPresent(mContractorID)) {
        warning() << "There is no contractor with requested id";
        return sendKeyErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    if (!mTrustLines->trustLineIsPresent(mContractorID)) {
        warning() << "Trust line is absent.";
        return sendKeyErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    // todo check contractor keys count
    info() << "Contractor keys count " << mContractorKeysCount;

    if (mCurrentKeyNumber != 0) {
        warning() << "Invalid init key number " << mCurrentKeyNumber;
        return sendKeyErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    if (mTrustLines->trustLineState(mContractorID) == TrustLine::Archived or
            mTrustLines->trustLineState(mContractorID) == TrustLine::Init) {
        warning() << "invalid TL state " << mTrustLines->trustLineState(mContractorID)
                  << ". Waiting for state updating";
        return sendKeyErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorID));
    try {
        mTrustLines->setIsContractorKeysPresent(mContractorID, false);
        keyChain.removeUnusedContractorKeys(ioTransaction);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't remove unused contractor keys. Details: " << e.what();
        throw e;
    }

    mStep = NextKeyProcessing;
    return runProcessKey(ioTransaction);
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::runReceiveNextKeyStage()
{
    info() << "runReceiveNextKeyStage";
    if (mContext.empty()) {
        warning() << "No next public key message received. Transaction will be closed.";
        return resultDone();
    }

    if (mTrustLines->trustLineState(mContractorID) == TrustLine::Archived) {
        warning() << "invalid TL state " << mTrustLines->trustLineState(mContractorID);
        return sendKeyErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    auto message = popNextMessage<PublicKeyMessage>();
    info() << "contractor " << message->idOnReceiverSide << " send next key.";
    if (message->idOnReceiverSide != mContractorID) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }

    if (message->number() != mCurrentKeyNumber + 1) {
        warning() << "Invalid key number " << message->number() << ". Wait for another";
        return resultContinuePreviousState();
    }

    info() << "Received key number " << message->number();

    mCurrentPublicKey = message->publicKey();
    mCurrentKeyNumber = message->number();

    auto ioTransaction = mStorageHandler->beginTransaction();
    return runProcessKey(ioTransaction);
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::runProcessKey(
    IOTransaction::Shared ioTransaction)
{
    info() << "runProcessKeyMessage";
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorID));
    try {
        keyChain.setContractorPublicKey(
            ioTransaction,
            mCurrentKeyNumber,
            mCurrentPublicKey);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceInitializationStage(
            BaseTransaction::PublicKeysSharingTargetTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceInitializationStage(
            BaseTransaction::PublicKeysSharingTargetTransactionType);
        mTrustLinesInfluenceController->testThrowExceptionOnTargetStage(
            BaseTransaction::PublicKeysSharingTargetTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnTargetStage(
            BaseTransaction::PublicKeysSharingTargetTransactionType);
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't store contractor public key. Details " << e.what();
        throw e;
    }
    info() << "Key saved, send hash confirmation";
    if (mCurrentKeyNumber == 0) {
        sendMessageWithTemporaryCaching<PublicKeyHashConfirmation>(
            mContractorID,
            Message::TrustLines_PublicKeysSharingInit,
            kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
            mEquivalent,
            mNodeUUID,
            mContractorsManager->idOnContractorSide(mContractorID),
            mTransactionUUID,
            mCurrentKeyNumber,
            mCurrentPublicKey->hash());
    } else {
        sendMessageWithTemporaryCaching<PublicKeyHashConfirmation>(
            mContractorID,
            Message::TrustLines_PublicKey,
            kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
            mEquivalent,
            mNodeUUID,
            mContractorsManager->idOnContractorSide(mContractorID),
            mTransactionUUID,
            mCurrentKeyNumber,
            mCurrentPublicKey->hash());
    }

    try {
        if (keyChain.allContractorKeysPresent(ioTransaction, mContractorKeysCount)) {
            info() << "All keys received";
            // todo maybe don't save TL state in storage only in memory (don't use ioTransaction and try catch)
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::Active,
                ioTransaction);
            mTrustLines->setIsContractorKeysPresent(
                mContractorID,
                true);
            if (mTrustLines->isTrustLineEmpty(mContractorID) and
                    !keyChain.ownKeysPresent(ioTransaction)) {
                info() << "publicKeysSharing Signal";
                publicKeysSharingSignal(
                    mContractorID,
                    mEquivalent);
            }
            info() << "TL is ready for using";
            return resultDone();
        }
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't update TL state. Details " << e.what();
        throw e;
    }
    return resultWaitForMessageTypes(
        {Message::TrustLines_PublicKey},
        kWaitMillisecondsForResponse * kMaxCountSendingAttempts);
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::sendKeyErrorConfirmation(
    ConfirmationMessage::OperationState errorState)
{
    sendMessage<PublicKeyHashConfirmation>(
        mContractorID,
        mEquivalent,
        mNodeUUID,
        mContractorsManager->idOnContractorSide(mContractorID),
        mTransactionUUID,
        errorState);
    return resultDone();
}

const string PublicKeysSharingTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[PublicKeySharingTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}