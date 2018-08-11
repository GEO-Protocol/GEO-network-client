#include "ConflictResolverInitiatorTransaction.h"

ConflictResolverInitiatorTransaction::ConflictResolverInitiatorTransaction(
    const NodeUUID &nodeUUID,
    SerializedEquivalent equivalent,
    const NodeUUID &contractorUUID,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger):

    BaseTransaction(
        BaseTransaction::ConflictResolverInitiatorTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

ConflictResolverInitiatorTransaction::ConflictResolverInitiatorTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :

    BaseTransaction(
        buffer,
        nodeUUID,
        logger),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{
    auto bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    mStep = Stages::Recovery;

    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
}

TransactionResult::SharedConst ConflictResolverInitiatorTransaction::run()
{
    switch (mStep) {
        case Stages::Initialization: {
            return runInitializationStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        case Stages::Recovery: {
            return runRecoveryStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep " + to_string(mStep));
    }
}

TransactionResult::SharedConst ConflictResolverInitiatorTransaction::runInitializationStage()
{
    info() << "runInitializationStage. Contractor " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Attempt to process not existing TL";
        return resultDone();
    }

    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::ConflictResolving) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
        // todo need correct reaction
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    mTrustLines->updateTrustLineFromStorage(
        mContractorUUID,
        ioTransaction);

    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(
            mContractorUUID));

    auto auditRecord = keyChain.actualFullAudit(
        ioTransaction);

    auto incomingReceipts = keyChain.incomingReceipts(
        ioTransaction,
        mTrustLines->auditNumber(mContractorUUID));
    auto outgoingReceipts = keyChain.outgoingReceipts(
        ioTransaction,
        mTrustLines->auditNumber(mContractorUUID));

    sendMessage<ConflictResolverMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        auditRecord,
        incomingReceipts,
        outgoingReceipts);

    info() << "Message with actual data was sent";

    mStep = Stages::ResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_ConflictResolverConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst ConflictResolverInitiatorTransaction::runResponseProcessingStage()
{
    info() << "runResponseProcessingStage";
    if (mContext.empty()) {
        warning() << "Contractor don't send response. Transaction will be closed, and wait for message";
        return resultDone();
    }
    auto response = popNextMessage<ConflictResolverResponseMessage>();
    if (response->senderUUID != mContractorUUID) {
        warning() << "Sender " << response->senderUUID << " is not contractor of this TA";
        return resultContinuePreviousState();
    }
    processConfirmationMessage(response);
    switch (response->state()) {
        case ConfirmationMessage::Audit_Reject: {
            info() << "Audit was rejected. Wait for contractor audit";
            // todo add checking if TL is become conflicted, maybe add new response state
            return resultDone();
        }
        case ConfirmationMessage::Audit_Invalid: {
            warning() << "Contractor send invalid audit response.";
            // todo : need correct reaction
            return resultDone();
        }
        case ConfirmationMessage::Audit_KeyNotFound: {
            warning() << "Contractor lost keys and can't accept audit";
            // todo : need correct reaction
            return resultDone();
        }
        case ConfirmationMessage::OK: {
            info() << "Contractor accept audit";
            auto ioTransaction = mStorageHandler->beginTransaction();
            mTrustLines->setTrustLineState(
                mContractorUUID,
                TrustLine::Active,
                ioTransaction);
            return resultDone();
        }
        default:
            warning() << "Invalid response state " << response->state();
            return resultDone();
    }
}

TransactionResult::SharedConst ConflictResolverInitiatorTransaction::runRecoveryStage()
{
    info() << "Recovery";
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent";
        return resultDone();
    }
    if (mTrustLines->trustLineState(mContractorUUID) == TrustLine::ConflictResolving) {
        info() << "Conflict resolving state";
        mStep = ResponseProcessing;
        return runResponseProcessingStage();
    }
    warning() << "Invalid TL state: " << mTrustLines->trustLineState(mContractorUUID);
    return resultDone();
}

pair<BytesShared, size_t> ConflictResolverInitiatorTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize;

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

    return make_pair(
        dataBytesShared,
        bytesCount);
}

const string ConflictResolverInitiatorTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[ConflictResolverInitiatorTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}