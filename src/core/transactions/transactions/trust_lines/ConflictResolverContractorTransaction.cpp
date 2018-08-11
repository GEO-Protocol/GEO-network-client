#include "ConflictResolverContractorTransaction.h"

ConflictResolverContractorTransaction::ConflictResolverContractorTransaction(
    const NodeUUID &nodeUUID,
    ConflictResolverMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::ConflictResolverContractorTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mContractorUUID(message->senderUUID),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

TransactionResult::SharedConst ConflictResolverContractorTransaction::run()
{
    info() << "initialized by " << mContractorUUID;
    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent.";
        sendMessage<ConflictResolverResponseMessage>(
            mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();

    bool isConflictResolvingSecondTime;
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::ConflictResolving) {
        info() << "TL on ConflictResolving state";
        isConflictResolvingSecondTime = true;
    } else {
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::ConflictResolving,
            ioTransaction);
        isConflictResolvingSecondTime = false;
    }
    mTrustLines->updateTrustLineFromStorage(
        mContractorUUID,
        ioTransaction);

    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(
            mContractorUUID));

    if (mTrustLines->auditNumber(mContractorUUID) > mMessage->auditRecord()->auditNumber()) {
        info() << "Our audit number is greater than contractor's";

        sendMessage<ConflictResolverResponseMessage>(
            mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            ConfirmationMessage::Audit_Reject);

        if (!isConflictResolvingSecondTime) {
            info() << "run ConflictResolverInitiatorTransaction signal";
            auto conflictResolverInitiatorTransaction = make_shared<ConflictResolverInitiatorTransaction>(
                mNodeUUID,
                mEquivalent,
                mContractorUUID,
                mTrustLines,
                mStorageHandler,
                mKeysStore,
                mTrustLinesInfluenceController,
                mLog);

            launchSubsidiaryTransaction(
                conflictResolverInitiatorTransaction);
        } else {
            warning() << "Conflict wasn't resolve";
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::Conflict,
                ioTransaction);
        }

        return resultDone();
    }

    if (mTrustLines->auditNumber(mContractorUUID) < mMessage->auditRecord()->auditNumber()) {
        info() << "Contractor's audit number is greater than ours";
        // check own signature
        try {
            if (!keyChain.checkOwnConflictedSignature(
                    ioTransaction,
                    mMessage->auditRecord()->serializeToCheckSignatureByContractor(),
                    AuditRecord::recordSizeForSignatureChecking(),
                    mMessage->auditRecord()->contractorSignature(),
                    mMessage->auditRecord()->contractorKeyHash())) {
                warning() << "Own audit data sent by contractor is incorrect";
                sendMessage<ConflictResolverResponseMessage>(
                    mContractorUUID,
                    mEquivalent,
                    mNodeUUID,
                    mTransactionUUID,
                    ConfirmationMessage::Audit_Invalid);
                return resultDone();
            }
        } catch (NotFoundError &e) {
            // todo need correct reaction, node can lost or remove own keys
            sendMessage<ConflictResolverResponseMessage>(
                mContractorUUID,
                mEquivalent,
                mNodeUUID,
                mTransactionUUID,
                ConfirmationMessage::Audit_KeyNotFound);
            return resultDone();
        } catch (IOError &e) {
            // todo need correct reaction
        }

        // check contractor signature
        try {
            if (!keyChain.checkContractorConflictedSignature(
                    ioTransaction,
                    mMessage->auditRecord()->serializeToCheckSignatureByInitiator(),
                    AuditRecord::recordSizeForSignatureChecking(),
                    mMessage->auditRecord()->ownSignature(),
                    mMessage->auditRecord()->ownKeyHash())) {
                warning() << "Own audit data sent by contractor is incorrect";
                sendMessage<ConflictResolverResponseMessage>(
                    mContractorUUID,
                    mEquivalent,
                    mNodeUUID,
                    mTransactionUUID,
                    ConfirmationMessage::Audit_Invalid);
                return resultDone();
            }
        } catch (NotFoundError &e) {
            // todo need correct reaction, node can lost or remove contractor keys
            sendMessage<ConflictResolverResponseMessage>(
                mContractorUUID,
                mEquivalent,
                mNodeUUID,
                mTransactionUUID,
                ConfirmationMessage::Audit_KeyNotFound);
            return resultDone();
        } catch (IOError &e) {
            // todo need correct reaction
        }

        // check own outgoing receipts signature
        for (const auto &incomingReceipt : mMessage->incomingReceipts()) {
            try {
                if (incomingReceipt->auditNumber() != mMessage->auditRecord()->auditNumber()) {
                    warning() << "Incoming receipt on TA " << incomingReceipt->transactionUUID()
                              << " sent by contractor has invalid audit number " << incomingReceipt->auditNumber();
                }
                auto serializedIncomingReceiptAndSize = getSerializedReceipt(
                    mNodeUUID,
                    mContractorUUID,
                    incomingReceipt);
                if (!keyChain.checkConflictedIncomingReceipt(
                        ioTransaction,
                        serializedIncomingReceiptAndSize.first,
                        serializedIncomingReceiptAndSize.second,
                        incomingReceipt->signature(),
                        incomingReceipt->keyHash())) {
                    warning() << "Incoming receipt on TA " << incomingReceipt->transactionUUID()
                              << " sent by contractor is incorrect";
                    sendMessage<ConflictResolverResponseMessage>(
                        mContractorUUID,
                        mEquivalent,
                        mNodeUUID,
                        mTransactionUUID,
                        ConfirmationMessage::Audit_Invalid);
                    return resultDone();
                }
            } catch (NotFoundError &e) {
                // todo need correct reaction, node can lost or remove contractor keys
                sendMessage<ConflictResolverResponseMessage>(
                    mContractorUUID,
                    mEquivalent,
                    mNodeUUID,
                    mTransactionUUID,
                    ConfirmationMessage::Audit_KeyNotFound);
                return resultDone();
            } catch (IOError &e) {
                // todo need correct reaction
            }
        }

        // check own incoming receipts signature
        for (const auto &outgoingReceipt : mMessage->outgoingReceipts()) {
            try {
                if (outgoingReceipt->auditNumber() != mMessage->auditRecord()->auditNumber()) {
                    warning() << "Outgoing receipt on TA " << outgoingReceipt->transactionUUID()
                              << " sent by contractor has invalid audit number " << outgoingReceipt->auditNumber();
                }
                auto serializedIncomingReceiptAndSize = getSerializedReceipt(
                    mContractorUUID,
                    mNodeUUID,
                    outgoingReceipt);
                if (!keyChain.checkConflictedOutgoingReceipt(
                        ioTransaction,
                        serializedIncomingReceiptAndSize.first,
                        serializedIncomingReceiptAndSize.second,
                        outgoingReceipt->signature(),
                        outgoingReceipt->keyHash())) {
                    warning() << "Outgoing receipt on TA " << outgoingReceipt->transactionUUID()
                              << " sent by contractor is incorrect";
                    sendMessage<ConflictResolverResponseMessage>(
                        mContractorUUID,
                        mEquivalent,
                        mNodeUUID,
                        mTransactionUUID,
                        ConfirmationMessage::Audit_Invalid);
                    return resultDone();
                }
            } catch (NotFoundError &e) {
                // todo need correct reaction, node can lost or remove contractor keys
                sendMessage<ConflictResolverResponseMessage>(
                    mContractorUUID,
                    mEquivalent,
                    mNodeUUID,
                    mTransactionUUID,
                    ConfirmationMessage::Audit_KeyNotFound);
                return resultDone();
            } catch (IOError &e) {
                // todo need correct reaction
            }
        }

        acceptContractorAuditData(
            ioTransaction,
            &keyChain);

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active,
            ioTransaction);

        sendMessage<ConflictResolverResponseMessage>(
            mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            ConfirmationMessage::OK);
    }

    info() << "Contractor's audit number is the same as ours";
    auto ownAuditRecord = keyChain.actualFullAudit(
        ioTransaction);

    bool isRunConflictResolverInitiatorTransaction = false;

    if (ownAuditRecord->incomingAmount() != mMessage->auditRecord()->outgoingAmount() or
            ownAuditRecord->outgoingAmount() != mMessage->auditRecord()->incomingAmount() or
            ownAuditRecord->balance() + mMessage->auditRecord()->balance() != TrustLine::kZeroBalance()) {
        warning() << "Contractor audit data is incorrect";
        // todo maybe check signatures???
        isRunConflictResolverInitiatorTransaction = true;
    }

    auto incomingReceipts = keyChain.incomingReceipts(
        ioTransaction,
        mTrustLines->auditNumber(mContractorUUID));

    auto outgoingReceipts = keyChain.outgoingReceipts(
        ioTransaction,
        mTrustLines->auditNumber(mContractorUUID));

    // check incoming receipts
    auto cntCommonReceipts = 0;
    for (const auto &contractorOutgoingReceiptRecord : mMessage->outgoingReceipts()) {
        bool isCommon = false;
        for (const auto &ownIncomingReceipt : incomingReceipts) {
            if (contractorOutgoingReceiptRecord == ownIncomingReceipt) {
                isCommon = true;
                break;
            }
        }
        if (isCommon) {
            cntCommonReceipts++;
        }
    }

    if (cntCommonReceipts < incomingReceipts.size()) {
        if (cntCommonReceipts == mMessage->outgoingReceipts().size()) {
            info() << "There are some incoming receipts which are absent on contractor side";
            isRunConflictResolverInitiatorTransaction = true;
        } else {
            info() << "There are different incoming receipts";
            // todo build actual data and send it to contractor
            isRunConflictResolverInitiatorTransaction = true;
        }
    } else if (cntCommonReceipts < mMessage->outgoingReceipts().size()) {
        info() << "There are some incoming receipts, which are absent on current node";
        // todo check these receipts and accept them
    }

    // check outgoing receipts
    cntCommonReceipts = 0;
    for (const auto &contractorIncomingReceiptRecord : mMessage->incomingReceipts()) {
        bool isCommon = false;
        for (const auto &ownOutgoingReceipt : outgoingReceipts) {
            if (contractorIncomingReceiptRecord == ownOutgoingReceipt) {
                isCommon = true;
                break;
            }
        }
        if (isCommon) {
            cntCommonReceipts++;
        }
    }

    if (cntCommonReceipts < outgoingReceipts.size()) {
        if (cntCommonReceipts == mMessage->incomingReceipts().size()) {
            info() << "There are some outgoing receipts which are absent on contractor side";
            isRunConflictResolverInitiatorTransaction = true;
        } else {
            info() << "There are different outgoing receipts";
            // todo build actual data and send it to contractor
            isRunConflictResolverInitiatorTransaction = true;
        }
    } else if (cntCommonReceipts < mMessage->incomingReceipts().size()) {
        info() << "There are some outgoing receipts, which are absent on current node";
        // todo check these receipts and accept them
    }

    if (isRunConflictResolverInitiatorTransaction) {
        info() << "Send Reject";
        sendMessage<ConflictResolverResponseMessage>(
           mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            ConfirmationMessage::Audit_Reject);

        if (!isConflictResolvingSecondTime) {
            info() << "run ConflictResolverInitiatorTransaction signal";
            auto conflictResolverInitiatorTransaction = make_shared<ConflictResolverInitiatorTransaction>(
                mNodeUUID,
                mEquivalent,
                mContractorUUID,
                mTrustLines,
                mStorageHandler,
                mKeysStore,
                mTrustLinesInfluenceController,
                mLog);
            launchSubsidiaryTransaction(
                conflictResolverInitiatorTransaction);
        } else {
            warning() << "Conflict wasn't resolve";
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::Conflict,
                ioTransaction);
        }
    } else {
        info() << "Send Ok";

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active,
            ioTransaction);

        sendMessage<ConflictResolverResponseMessage>(
            mContractorUUID,
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            ConfirmationMessage::OK);
    }

    return resultDone();
}

pair<BytesShared, size_t> ConflictResolverContractorTransaction::getSerializedReceipt(
    const NodeUUID &source,
    const NodeUUID &target,
    const ReceiptRecord::Shared receiptRecord)
{
    size_t serializedDataSize = NodeUUID::kBytesSize
                                + NodeUUID::kBytesSize
                                + TransactionUUID::kBytesSize
                                + kTrustLineAmountBytesCount
                                + sizeof(AuditNumber);
    BytesShared serializedData = tryMalloc(serializedDataSize);

    size_t bytesBufferOffset = 0;
    memcpy(
        serializedData.get() + bytesBufferOffset,
        source.data,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    memcpy(
        serializedData.get() + bytesBufferOffset,
        target.data,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    memcpy(
        serializedData.get() + bytesBufferOffset,
        receiptRecord->transactionUUID().data,
        TransactionUUID::kBytesSize);
    bytesBufferOffset += TransactionUUID::kBytesSize;

    auto serializedAmount = trustLineAmountToBytes(receiptRecord->amount());
    memcpy(
        serializedData.get() + bytesBufferOffset,
        serializedAmount.data(),
        kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    auto auditNumber = receiptRecord->auditNumber();
    memcpy(
        serializedData.get() + bytesBufferOffset,
        &auditNumber,
        sizeof(AuditNumber));

    return make_pair(
        serializedData,
        serializedDataSize);
}

void ConflictResolverContractorTransaction::acceptContractorAuditData(
    IOTransaction::Shared ioTransaction,
    TrustLineKeychain *keyChain)
{
    keyChain->acceptAudit(
        ioTransaction,
        mMessage->auditRecord());

    keyChain->acceptReceipts(
        ioTransaction,
        mMessage->incomingReceipts(),
        mMessage->outgoingReceipts());

    mTrustLines->updateTrustLineFromStorage(
        mContractorUUID,
        ioTransaction);
}

const string ConflictResolverContractorTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[ConflictResolverContractorTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}