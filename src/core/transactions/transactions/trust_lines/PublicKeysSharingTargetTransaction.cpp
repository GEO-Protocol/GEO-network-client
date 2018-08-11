#include "PublicKeysSharingTargetTransaction.h"

PublicKeysSharingTargetTransaction::PublicKeysSharingTargetTransaction(
    const NodeUUID &nodeUUID,
    PublicKeyMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :
    BaseTrustLineTransaction(
        BaseTransaction::PublicKeysSharingTargetTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        message->senderUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger)
{
    mCurrentKeyNumber = message->number();
    mCurrentPublicKey = message->publicKey();
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
    mStep = KeysSharingTargetInitialization;
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::run()
{
    switch (mStep) {
        case Stages::KeysSharingTargetInitialization: {
            mStep = KeysSharingTargetNextKey;
            return runPublicKeyReceiverStage();
        }
        case Stages::KeysSharingTargetNextKey: {
            return runReceiveNextKeyStage();
        }
        case Stages::KeysSharingInitialization: {
            return runPublicKeysSharingInitializationStage();
        }
        case Stages::NextKeyProcessing: {
            return runPublicKeysSendNextKeyStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                "wrong value of mStep " + to_string(mStep));
    }
}

TransactionResult::SharedConst PublicKeysSharingTargetTransaction::runReceiveNextKeyStage()
{
    info() << "runReceiveNextKeyStage";
    if (mContext.empty()) {
        warning() << "No next public key message received. Transaction will be closed, and wait for message";
        return resultDone();
    }
    auto message = popNextMessage<PublicKeyMessage>();
    mCurrentPublicKey = message->publicKey();
    mCurrentKeyNumber = message->number();
    return runPublicKeyReceiverStage();
}

const string PublicKeysSharingTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[PublicKeySharingTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}