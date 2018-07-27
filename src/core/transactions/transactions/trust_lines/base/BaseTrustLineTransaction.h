#ifndef GEO_NETWORK_CLIENT_BASETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRUSTLINETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../crypto/keychain.h"
#include "../../../../crypto/lamportkeys.h"

#include "../../../../subsystems_controller/TrustLinesInfluenceController.h"

#include "../../../../network/messages/trust_lines/TrustLineConfirmationMessage.h"
#include "../../../../network/messages/trust_lines/AuditMessage.h"
#include "../../../../network/messages/trust_lines/PublicKeyMessage.h"
#include "../../../../network/messages/trust_lines/PublicKeyHashConfirmation.h"

class BaseTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<BaseTrustLineTransaction> Shared;

    BaseTrustLineTransaction(
        const TransactionType type,
        const NodeUUID &currentNodeUUID,
        const SerializedEquivalent equivalent,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &log);

    BaseTrustLineTransaction(
        const TransactionType type,
        const TransactionUUID &transactionUUID,
        const NodeUUID &currentNodeUUID,
        const SerializedEquivalent equivalent,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &log);

    BaseTrustLineTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &log);

protected:
    enum Stages {
        TrustLineInitialisation = 1,
        TrustLineResponseProcessing = 2,
        AuditInitialisation = 3,
        AuditResponseProcessing = 4,
        AuditTarget = 5,
        KeysSharingInitialization = 6,
        NextKeyProcessing = 7,
        KeysSharingTargetInitialization = 8,
        KeysSharingTargetNextKey = 9,
        Recovery = 10,
        AddToBlackList = 11,
    };

protected:
    TransactionResult::SharedConst runPublicKeysSharingInitialisationStage();

    TransactionResult::SharedConst runPublicKeysSendNextKeyStage();

    TransactionResult::SharedConst runPublicKeyReceiverStage();

    TransactionResult::SharedConst runAuditInitializationStage();

    TransactionResult::SharedConst runAuditResponseProcessingStage();

    TransactionResult::SharedConst runAuditTargetStage();

    void updateTrustLineStateAfterInitialAudit(
        IOTransaction::Shared ioTransaction);

    void updateTrustLineStateAfterNextAudit(
        IOTransaction::Shared ioTransaction,
        TrustLineKeychain *keyChain);

    TransactionResult::SharedConst sendAuditErrorConfirmation(
        ConfirmationMessage::OperationState errorState);

    TransactionResult::SharedConst sendTrustLineErrorConfirmation(
        ConfirmationMessage::OperationState errorState);

protected:
    pair<BytesShared, size_t> getOwnSerializedAuditData();

    pair<BytesShared, size_t> getContractorSerializedAuditData();

protected:
    static const uint32_t kWaitMillisecondsForResponse = 60000;

protected:
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;

    NodeUUID mContractorUUID;
    AuditNumber mAuditNumber;
    pair<lamport::Signature::Shared, KeyNumber> mOwnSignatureAndKeyNumber;

    AuditMessage::Shared mAuditMessage;

    KeyNumber mCurrentKeyNumber;
    lamport::PublicKey::Shared mCurrentPublicKey;

    TrustLinesInfluenceController *mTrustLinesInfluenceController;
};


#endif //GEO_NETWORK_CLIENT_BASETRUSTLINETRANSACTION_H
