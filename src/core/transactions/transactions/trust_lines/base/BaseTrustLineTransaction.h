#ifndef GEO_NETWORK_CLIENT_BASETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRUSTLINETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../crypto/keychain.h"
#include "../../../../crypto/lamportkeys.h"

#include "../../../../subsystems_controller/TrustLinesInfluenceController.h"

#include "../../../../network/messages/trust_lines/TrustLineConfirmationMessage.h"
#include "../../../../network/messages/trust_lines/AuditMessage.h"
#include "../../../../network/messages/trust_lines/AuditResponseMessage.h"
#include "../../../../network/messages/trust_lines/PublicKeysSharingInitMessage.h"
#include "../../../../network/messages/trust_lines/PublicKeyMessage.h"
#include "../../../../network/messages/trust_lines/PublicKeyHashConfirmation.h"

#include "../ConflictResolverInitiatorTransaction.h"

namespace signals = boost::signals2;

class BaseTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<BaseTrustLineTransaction> Shared;

    BaseTrustLineTransaction(
        const TransactionType type,
        const NodeUUID &currentNodeUUID,
        const SerializedEquivalent equivalent,
        const NodeUUID &contractorUUID,
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
        const NodeUUID &contractorUUID,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &log);

protected:
    enum Stages {
        Initialization = 1,
        ResponseProcessing = 2,
        AddToBlackList = 11,
    };

protected:
    TransactionResult::SharedConst sendAuditErrorConfirmation(
        ConfirmationMessage::OperationState errorState);

    pair<BytesShared, size_t> getOwnSerializedAuditData();

    pair<BytesShared, size_t> getContractorSerializedAuditData();

public:
    mutable TrustLineActionSignal trustLineActionSignal;

protected:
    static const uint32_t kWaitMillisecondsForResponse = 60000;

protected:
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;

    NodeUUID mContractorUUID;
    AuditNumber mAuditNumber;
    pair<lamport::Signature::Shared, KeyNumber> mOwnSignatureAndKeyNumber;

    TrustLinesInfluenceController *mTrustLinesInfluenceController;
};


#endif //GEO_NETWORK_CLIENT_BASETRUSTLINETRANSACTION_H
