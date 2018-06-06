#include "PublicKeysSharingSourceTransaction.h"

PublicKeysSharingSourceTransaction::PublicKeysSharingSourceTransaction(
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::PublicKeysSharingSourceTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeyChain(KeyChain::makeKeyChain(manager->trustLineReadOnly(contractorUUID)->trustLineID(), logger))
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
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mKeyChain.initGeneration(kKeysCount, ioTransaction);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't generate public keys. Details: " << e.what();
        return resultDone();
    }
    info() << "All keys saved";
    for (const auto &numberAndKey : mKeyChain.allAvailablePublicKeys(ioTransaction)) {
        mPublicKeys.insert(
            make_pair(
                numberAndKey.first,
                numberAndKey.second));
    }
    mCurrentKey = mPublicKeys.begin();
    info() << "before sending";
    sendMessage<PublicKeyMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        mCurrentKey->first,
        mCurrentKey->second);
    info() << "Send key number: " << mCurrentKey->first << " key: " << (int)*mCurrentKey->second.key()
           << " key size: " << mCurrentKey->second.keySize();
    mStep = SendNextKey;
    return resultWaitForMessageTypes(
        {Message::TrustLines_CRCConfirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst PublicKeysSharingSourceTransaction::runSendNextKeyStage()
{
    if (mContext.empty()) {
        warning() << "No confirmation message received";
        return resultDone();
    }
    auto message = popNextMessage<PublicKeyCRCConfirmation>();
    if (message->number() != mCurrentKey->first || message->crcConfirmation() != mCurrentKey->second.crc()) {
        warning() << "Number or CRC is incorrect";
        return resultDone();
    }
    info() << "Key number: " << mCurrentKey->first << " key: " << (int)*mCurrentKey->second.key() << " confirmed";
    mCurrentKey++;
    if (mCurrentKey == mPublicKeys.end()) {
        info() << "all keys confirmed";
        auto ioTransaction = mStorageHandler->beginTransaction();
        if (mKeyChain.isAllKeysReady(ioTransaction, kKeysCount)) {
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

    sendMessage<PublicKeyMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        currentTransactionUUID(),
        mCurrentKey->first,
        mCurrentKey->second);
    info() << "Send key number: " << mCurrentKey->first << " key: " << (int)*mCurrentKey->second.key();
    return resultWaitForMessageTypes(
        {Message::TrustLines_CRCConfirmation},
        kWaitMillisecondsForResponse);
}

const string PublicKeysSharingSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[PublicKeysSharingSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}