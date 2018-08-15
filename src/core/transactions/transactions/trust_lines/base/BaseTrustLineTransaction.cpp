#include "BaseTrustLineTransaction.h"

BaseTrustLineTransaction::BaseTrustLineTransaction(
    const TransactionType type,
    const NodeUUID &currentNodeUUID,
    const SerializedEquivalent equivalent,
    const NodeUUID &contractorUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &log) :

    BaseTransaction(
        type,
        currentNodeUUID,
        equivalent,
        log),
    mContractorUUID(contractorUUID),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

BaseTrustLineTransaction::BaseTrustLineTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &currentNodeUUID,
    const SerializedEquivalent equivalent,
    const NodeUUID &contractorUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &log) :

    BaseTransaction(
        type,
        transactionUUID,
        currentNodeUUID,
        equivalent,
        log),
    mContractorUUID(contractorUUID),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

BaseTrustLineTransaction::BaseTrustLineTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &log):
    BaseTransaction(
        buffer,
        nodeUUID,
        log),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

TransactionResult::SharedConst BaseTrustLineTransaction::runAuditInitializationStage()
{
    info() << "runAuditInitializationStage Contractor " << mContractorUUID;
    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::AuditPending) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
        // todo implement actual reaction
        return resultDone();
    }

    auto serializedAuditData = getOwnSerializedAuditData();
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
            mTrustLines->trustLineID(mContractorUUID));
    try {
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnAuditStage();
        mTrustLinesInfluenceController->testTerminateProcessOnAuditStage();
#endif

        auto bytesAndCount = serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);
        info() << "Transaction saved";
    } catch(IOError &e) {
        ioTransaction->rollback();
        error() << "Can't sign audit data. Details: " << e.what();
        throw e;
    }

    sendMessage<AuditMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;

    mStep = AuditResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_AuditConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst BaseTrustLineTransaction::runAuditResponseProcessingStage()
{
    info() << "runAuditResponseProcessingStage";
    if (mContext.empty()) {
        warning() << "No audit confirmation message received. Transaction will be closed, and wait for message";
        return resultDone();
    }

    auto message = popNextMessage<AuditResponseMessage>();
    info() << "Contractor send audit message";
    if (message->senderUUID != mContractorUUID) {
        warning() << "Receive message from different sender: " << message->senderUUID;
        return resultContinuePreviousState();
    }

    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Something wrong, because TL must be present";
        // todo : need correct reaction
        return resultDone();
    }

    if (message->state() == ConfirmationMessage::ReservationsPresentOnTrustLine) {
        info() << "Contractor's TL is not ready for audit yet";
        // message on communicator queue, wait for audit response after reservations committing or cancelling
        // todo add timeout or count failed attempts for running conflict resolver TA
        return resultWaitForMessageTypes(
            {Message::TrustLines_AuditConfirmation},
            kWaitMillisecondsForResponse);
    }

    processConfirmationMessage(message);
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));

    auto contractorSerializedAuditData = getContractorSerializedAuditData();
    try {
        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept audit. Response code: " << message->state();
            // delete this transaction from storage
            ioTransaction->transactionHandler()->deleteRecord(
                currentTransactionUUID());

            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::ConflictResolving,
                ioTransaction);
            auto conflictResolverTransaction = make_shared<ConflictResolverInitiatorTransaction>(
                mNodeUUID,
                mEquivalent,
                mContractorUUID,
                mTrustLines,
                mStorageHandler,
                mKeysStore,
                mTrustLinesInfluenceController,
                mLog);
            info() << "Launch ConflictResolverInitiatorTransaction signal";
            launchSubsidiaryTransaction(
                conflictResolverTransaction);
            return resultDone();
        }

        if (!keyChain.checkSign(
                ioTransaction,
                contractorSerializedAuditData.first,
                contractorSerializedAuditData.second,
                message->signature(),
                message->keyNumber())) {
            warning() << "Contractor didn't sign message correct";
            // delete this transaction from storage
            ioTransaction->transactionHandler()->deleteRecord(
                currentTransactionUUID());
            // todo run conflict resolver TA
            return resultDone();
        }
        info() << "Signature is correct";

#ifdef TETST
        mTrustLinesInfluenceController->testThrowExceptionOnAuditResponseProcessingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnAuditResponseProcessingStage();
#endif

        keyChain.saveAudit(
            ioTransaction,
            mAuditNumber,
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first,
            message->keyNumber(),
            message->signature(),
            mTrustLines->incomingTrustAmount(
                mContractorUUID),
            mTrustLines->outgoingTrustAmount(
                mContractorUUID),
            mTrustLines->balance(mContractorUUID));

        if (mAuditNumber == TrustLine::kInitialAuditNumber) {
            updateTrustLineStateAfterInitialAudit(
                ioTransaction);
        } else {
            updateTrustLineStateAfterNextAudit(
                ioTransaction,
                &keyChain);
        }
        info() << "TL state updated";

        // delete this transaction from storage
        ioTransaction->transactionHandler()->deleteRecord(
            currentTransactionUUID());
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't check signature, update TL on storage or save Audit. Details: " << e.what();
        throw e;
    }

    info() << "All data saved. Now TL is ready for using";
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        mStep = KeysSharingInitialization;
        return resultAwakeAsFastAsPossible();
    }
    return resultDone();
}

TransactionResult::SharedConst BaseTrustLineTransaction::runAuditTargetStage()
{
    info() << "runAuditTargetStage by " << mContractorUUID;
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust Line is absent";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::AuditPending) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));

    auto contractorSerializedAuditData = getContractorSerializedAuditData();
    try {
        if (!keyChain.checkSign(
                ioTransaction,
                contractorSerializedAuditData.first,
                contractorSerializedAuditData.second,
                mAuditMessage->signature(),
                mAuditMessage->keyNumber())) {
            warning() << "Contractor didn't sign message correct";
            return sendAuditErrorConfirmation(
                ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        }
        info() << "Signature is correct";
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't update TL on storage. Details: " << e.what();
        throw e;
    }

    auto serializedAuditData = getOwnSerializedAuditData();
    try {
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnAuditStage();
        mTrustLinesInfluenceController->testTerminateProcessOnAuditStage();
#endif

        keyChain.saveAudit(
            ioTransaction,
            mAuditNumber,
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first,
            mAuditMessage->keyNumber(),
            mAuditMessage->signature(),
            mTrustLines->incomingTrustAmount(
                mContractorUUID),
            mTrustLines->outgoingTrustAmount(
                mContractorUUID),
            mTrustLines->balance(mContractorUUID));

        if (mAuditNumber == TrustLine::kInitialAuditNumber) {
            updateTrustLineStateAfterInitialAudit(
                ioTransaction);
        } else {
            updateTrustLineStateAfterNextAudit(
                ioTransaction,
                &keyChain);
        }
        info() << "TL state updated";

        // delete this transaction from storage
        ioTransaction->transactionHandler()->deleteRecord(
            currentTransactionUUID());
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't sign audit data. Details: " << e.what();
        throw e;
    }
    info() << "All data saved. Now TL is ready for using";

    sendMessageWithCaching<AuditResponseMessage>(
        mContractorUUID,
        Message::TrustLines_Audit,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;

    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::KeysPending) {
        mStep = KeysSharingInitialization;
        return resultAwakeAsFastAsPossible();
    }
    return resultDone();
}

void BaseTrustLineTransaction::updateTrustLineStateAfterInitialAudit(
    IOTransaction::Shared ioTransaction)
{
    info() << "updateTrustLineStateAfterInitialAudit";
    try {
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active,
            ioTransaction);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't save Audit or update TL on storage. Details: " << e.what();
        throw e;
    }
}

void BaseTrustLineTransaction::updateTrustLineStateAfterNextAudit(
    IOTransaction::Shared ioTransaction,
    TrustLineKeychain *keyChain)
{
    info() << "updateTrustLineStateAfterNextAudit";
    try{
        mTrustLines->setTrustLineAuditNumberAndMakeActive(
            ioTransaction,
            mContractorUUID,
            mAuditNumber);
        mTrustLines->resetTrustLineTotalReceiptsAmounts(
            mContractorUUID);
        if (mTrustLines->isTrustLineEmpty(
            mContractorUUID)) {
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::Archived,
                ioTransaction);
            info() << "TL is Archived";
        } else if (keyChain->ownKeysCriticalCount(ioTransaction)) {
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::KeysPending,
                ioTransaction);
            info() << "Start sharing own keys";
        }
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't update TL on storage. Details: " << e.what();
        throw e;
    }
}

TransactionResult::SharedConst BaseTrustLineTransaction::runPublicKeysSharingInitializationStage()
{
    info() << "runPublicKeysSharingInitializationStage with " << mContractorUUID;

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

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnKeysSharingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnKeysSharingStage();
#endif

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
        mTransactionUUID,
        mCurrentKeyNumber,
        mCurrentPublicKey);
    info() << "Send key number: " << mCurrentKeyNumber;

    mStep = NextKeyProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_HashConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst BaseTrustLineTransaction::runPublicKeysSendNextKeyStage()
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
    processConfirmationMessage(message);
    mCurrentKeyNumber++;
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    if (mCurrentKeyNumber >= TrustLineKeychain::kDefaultKeysSetSize) {
        info() << "all keys confirmed";
        auto ioTransaction = mStorageHandler->beginTransaction();
        try {
            if (mAuditNumber == TrustLine::kInitialAuditNumber) {
                auto bytesAndCount = serializeToBytes();
                info() << "Transaction serialized";
                ioTransaction->transactionHandler()->saveRecord(
                    currentTransactionUUID(),
                    bytesAndCount.first,
                    bytesAndCount.second);
                info() << "Transaction saved";
                if (keyChain.contractorKeysPresent(ioTransaction)) {
                    mTrustLines->setTrustLineState(
                        mContractorUUID,
                        TrustLine::AuditPending,
                        ioTransaction);
                    mStep = AuditTarget;
                    info() << "Waiting for initial audit";
                    return resultWaitForMessageTypes(
                        {Message::TrustLines_Audit},
                        kWaitMillisecondsForResponse);
                } else {
                    info() << "Waiting for contractor public keys";
                    mStep = KeysSharingTargetNextKey;
                    return resultWaitForMessageTypes(
                        {Message::TrustLines_PublicKey},
                        kWaitMillisecondsForResponse);
                }
            } else {
                mTrustLines->setTrustLineState(
                    mContractorUUID,
                    TrustLine::Active,
                    ioTransaction);
                // delete this transaction from storage
                ioTransaction->transactionHandler()->deleteRecord(
                    currentTransactionUUID());
                info() << "TL is ready for using";
            }
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

        auto bytesAndCount = serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            currentTransactionUUID(),
            bytesAndCount.first,
            bytesAndCount.second);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnKeysSharingProcessingResponseStage();
        mTrustLinesInfluenceController->testTerminateProcessOnKeysSharingProcessingResponseStage();
#endif

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

TransactionResult::SharedConst BaseTrustLineTransaction::runPublicKeyReceiverStage()
{
    info() << "runPublicKeyReceiverStage";
    info() << "Receive key number: " << mCurrentKeyNumber << " from " << mContractorUUID;

    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent.";
        return sendKeyErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
            mTrustLines->trustLineID(mContractorUUID));
    try {
        if (mCurrentKeyNumber == 0) {
            info() << "init key number";
            if (mAuditNumber == TrustLine::kInitialAuditNumber) {
                if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Init
                    and mTrustLines->trustLineState(mContractorUUID) != TrustLine::KeysPending) {
                    warning() << "invalid TL state " << mTrustLines->trustLineState(mContractorUUID)
                              << " for init TL. Waiting for state updating";
                    return resultDone();
                }
            } else {
                if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Active) {
                    warning() << "invalid TL state " << mTrustLines->trustLineState(mContractorUUID)
                              << ". Waiting for state updating";
                    return resultDone();
                }
            }
            keyChain.removeUnusedContractorKeys(ioTransaction);
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::KeysPending,
                ioTransaction);
        } else {
            if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::KeysPending) {
                warning() << "invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
                return sendKeyErrorConfirmation(
                    ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
            }
        }
        keyChain.setContractorPublicKey(
            ioTransaction,
            mCurrentKeyNumber,
            mCurrentPublicKey);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnKeysSharingReceiverStage();
        mTrustLinesInfluenceController->testTerminateProcessOnKeysSharingReceiverStage();
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't store contractor public key. Details " << e.what();
        throw e;
    }
    info() << "Key saved, send hash confirmation";
    sendMessageWithCaching<PublicKeyHashConfirmation>(
        mContractorUUID,
        Message::TrustLines_PublicKey,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mCurrentKeyNumber,
        mCurrentPublicKey->hash());

    if (keyChain.allContractorKeysPresent(ioTransaction)) {
        info() << "All keys received";
        // todo check if count of keys are equal on both sides: source and target
        // maybe add new message
        try {
            if (mAuditNumber == TrustLine::kInitialAuditNumber) {
                if (keyChain.ownKeysPresent(ioTransaction)) {
                    mTrustLines->setTrustLineState(
                        mContractorUUID,
                        TrustLine::AuditPending,
                        ioTransaction);
                    info() << "Start initial audit";
                    mStep = AuditInitialization;
                    return resultAwakeAsFastAsPossible();
                } else {
                    info() << "Start sharing own keys";
                    mStep = KeysSharingInitialization;
                    return resultAwakeAsFastAsPossible();
                }
            } else {
                mTrustLines->setTrustLineState(
                    mContractorUUID,
                    TrustLine::Active,
                    ioTransaction);
                // delete this transaction from storage
                ioTransaction->transactionHandler()->deleteRecord(
                    currentTransactionUUID());
                info() << "TL is ready for using";
                return resultDone();
            }
        } catch (IOError &e) {
            ioTransaction->rollback();
            error() << "Can't update TL state. Details " << e.what();
            throw e;
        }
    }
    return resultWaitForMessageTypes(
        {Message::TrustLines_PublicKey},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst BaseTrustLineTransaction::sendTrustLineErrorConfirmation(
    ConfirmationMessage::OperationState errorState)
{
    sendMessage<TrustLineConfirmationMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        false,
        errorState);
    return resultDone();
}

TransactionResult::SharedConst BaseTrustLineTransaction::sendKeyErrorConfirmation(
    ConfirmationMessage::OperationState errorState)
{
    sendMessage<PublicKeyHashConfirmation>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        errorState);
    return resultDone();
}

TransactionResult::SharedConst BaseTrustLineTransaction::sendAuditErrorConfirmation(
    ConfirmationMessage::OperationState errorState)
{
    sendMessage<AuditResponseMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        errorState);
    return resultDone();
}

pair<BytesShared, size_t> BaseTrustLineTransaction::getOwnSerializedAuditData()
{
    size_t bytesCount = sizeof(AuditNumber)
                        + kTrustLineAmountBytesCount
                        + kTrustLineAmountBytesCount
                        + kTrustLineBalanceSerializeBytesCount;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);
    info() << "own audit " << mAuditNumber;

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->incomingTrustAmount(
            mContractorUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "own incoming amount " << mTrustLines->incomingTrustAmount(mContractorUUID);

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->outgoingTrustAmount(
            mContractorUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "own outgoing amount " << mTrustLines->outgoingTrustAmount(mContractorUUID);

    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(mTrustLines->balance(mContractorUUID)));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);
    info() << "own balance " << mTrustLines->balance(mContractorUUID);

    return make_pair(
        dataBytesShared,
        bytesCount);
}

pair<BytesShared, size_t> BaseTrustLineTransaction::getContractorSerializedAuditData()
{
    size_t bytesCount = sizeof(AuditNumber)
                        + kTrustLineAmountBytesCount
                        + kTrustLineAmountBytesCount
                        + kTrustLineBalanceSerializeBytesCount;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);
    info() << "contractor audit " << mAuditNumber;

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->outgoingTrustAmount(
            mContractorUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "contractor outgoing amount " << mTrustLines->outgoingTrustAmount(mContractorUUID);

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mTrustLines->incomingTrustAmount(
            mContractorUUID));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
    info() << "contractor incoming amount " << mTrustLines->incomingTrustAmount(mContractorUUID);

    auto contractorBalance = -1 * mTrustLines->balance(mContractorUUID);
    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(contractorBalance));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);
    info() << "contractor balance " << contractorBalance;

    return make_pair(
        dataBytesShared,
        bytesCount);
}