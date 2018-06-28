#include "AuditSourceTransaction.h"

AuditSourceTransaction::AuditSourceTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::AuditSourceTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mAuditNumber(mTrustLines->auditNumber(mContractorUUID) + 1)
{}

TransactionResult::SharedConst AuditSourceTransaction::run()
{
    switch (mStep) {
        case Stages::Initialisation: {
            return runInitialisationStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                    "wrong value of mStep");
    }
}

TransactionResult::SharedConst AuditSourceTransaction::runInitialisationStage()
{
    auto serializedAuditData = getOwnSerializedAuditData();
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    try {
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);
    } catch(IOError &e) {
        ioTransaction->rollback();
        error() << "Can't sign audit data. Details: " << e.what();
        return resultDone();
    }

    sendMessage<AuditMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;
    mStep = ResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::TrustLines_Audit},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst AuditSourceTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "No audit confirmation message received";
        return resultDone();
    }

    auto message = popNextMessage<AuditMessage>();
    info() << "Contractor send audit message";
    if (message->senderUUID != mContractorUUID) {
        warning() << "Receive message from different sender: " << message->senderUUID;
        return resultContinuePreviousState();
    }
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));

    auto contractorSerializedAuditData = getContractorSerializedAuditData();
    if (!keyChain.checkSign(
        ioTransaction,
        contractorSerializedAuditData.first,
        contractorSerializedAuditData.second,
        message->signature(),
        message->keyNumber())) {
        warning() << "Contractor didn't sign message correct";
        // todo run conflict resolver TA
        return resultDone();
    }
    info() << "Signature is correct";
    try {
        mTrustLines->setTrustLineAuditNumber(
            ioTransaction,
            mContractorUUID,
            mAuditNumber);
        // if TL is empty: incomingAmount, outgoingAmount and balance is 0
        // we marked it as Archived
        if (mTrustLines->isTrustLineEmpty(
            mContractorUUID)) {
            mTrustLines->setTrustLineState(
                ioTransaction,
                mContractorUUID,
                TrustLine::Archived);
        }
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't save Audit or update TL on storage. Details: " << e.what();
        return resultDone();
    }
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
    info() << "All data saved. Now TL is ready for using";
    return resultDone();
}

pair<BytesShared, size_t> AuditSourceTransaction::getOwnSerializedAuditData()
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

pair<BytesShared, size_t> AuditSourceTransaction::getContractorSerializedAuditData()
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

const string AuditSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[AuditSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}