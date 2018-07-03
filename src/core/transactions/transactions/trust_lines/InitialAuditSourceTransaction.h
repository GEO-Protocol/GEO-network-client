#ifndef GEO_NETWORK_CLIENT_INITIALAUDITSOURCETRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIALAUDITSOURCETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/keychain.h"
#include "../../../crypto/lamportkeys.h"

#include "../../../network/messages/trust_lines/InitialAuditMessage.h"

using namespace crypto;

class InitialAuditSourceTransaction : public BaseTransaction {

public:
    typedef shared_ptr<InitialAuditSourceTransaction> Shared;

public:
    InitialAuditSourceTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        const SerializedEquivalent equivalent,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        Logger &logger);

    InitialAuditSourceTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        Initialisation = 1,
        ResponseProcessing = 2,
        Recovery = 3,
    };

protected: // log
    const string logHeader() const;

private:
    TransactionResult::SharedConst runInitialisationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

    TransactionResult::SharedConst runRecoveryStage();

    pair<BytesShared, size_t> serializeToBytes() const override;

    pair<BytesShared, size_t> getOwnSerializedAuditData();

    pair<BytesShared, size_t> getContractorSerializedAuditData();

private:
    static const uint32_t kWaitMillisecondsForResponse = 60000;

    static const AuditNumber kInitialAuditNumber = 0;

protected:
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;
    pair<lamport::Signature::Shared, KeyNumber> mOwnSignatureAndKeyNumber;
};


#endif //GEO_NETWORK_CLIENT_INITIALAUDITSOURCETRANSACTION_H
