#include "ConflictResolverInitiatorTransaction.h"

ConflictResolverInitiatorTransaction::ConflictResolverInitiatorTransaction(
    SerializedEquivalent equivalent,
    ContractorID contractorID,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger):

    BaseTransaction(
        BaseTransaction::ConflictResolverInitiatorTransactionType,
        equivalent,
        logger),
    mContractorID(contractorID),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

ConflictResolverInitiatorTransaction::ConflictResolverInitiatorTransaction(
    BytesShared buffer,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :

    BaseTransaction(
        buffer,
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{
    auto bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    mStep = Stages::Recovery;

    memcpy(
        &mContractorID,
        buffer.get() + bytesBufferOffset,
        sizeof(ContractorID));
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
    info() << "runInitializationStage. Contractor " << mContractorID;

    if (!mContractorsManager->contractorPresent(mContractorID)) {
        warning() << "There is no contractor with requested id";
        return resultDone();
    }

    if (!mTrustLinesManager->trustLineIsPresent(mContractorID)) {
        warning() << "Attempt to process not existing TL";
        return resultDone();
    }

    if (mTrustLinesManager->trustLineState(mContractorID) != TrustLine::ConflictResolving) {
        warning() << "Invalid TL state " << mTrustLinesManager->trustLineState(mContractorID);
        // todo need correct reaction
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    mTrustLinesManager->updateTrustLineFromStorage(
        mContractorID,
        ioTransaction);

    auto keyChain = mKeysStore->keychain(
        mTrustLinesManager->trustLineID(
            mContractorID));

    auto auditRecord = keyChain.actualFullAudit(
        ioTransaction);

    auto incomingReceipts = keyChain.incomingReceipts(
        ioTransaction,
        mTrustLinesManager->auditNumber(mContractorID));
    auto outgoingReceipts = keyChain.outgoingReceipts(
        ioTransaction,
        mTrustLinesManager->auditNumber(mContractorID));

    sendMessage<ConflictResolverMessage>(
        mContractorID,
        mEquivalent,
        mContractorsManager->idOnContractorSide(mContractorID),
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
    if (response->idOnReceiverSide != mContractorID) {
        warning() << "Sender " << response->idOnReceiverSide << " is not contractor of this TA";
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
            mTrustLinesManager->setTrustLineState(
                mContractorID,
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
    if (!mTrustLinesManager->trustLineIsPresent(mContractorID)) {
        warning() << "Trust line is absent";
        return resultDone();
    }
    if (mTrustLinesManager->trustLineState(mContractorID) == TrustLine::ConflictResolving) {
        info() << "Conflict resolving state";
        mStep = ResponseProcessing;
        return runResponseProcessingStage();
    }
    warning() << "Invalid TL state: " << mTrustLinesManager->trustLineState(mContractorID);
    return resultDone();
}

pair<BytesShared, size_t> ConflictResolverInitiatorTransaction::serializeToBytes() const
{
    const auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(ContractorID);

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mContractorID,
        sizeof(ContractorID));

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