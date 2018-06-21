#ifndef GEO_NETWORK_CLIENT_INITIALAUDITSOURCETRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIALAUDITSOURCETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/keychain.h"
#include "../../../crypto/lamportkeys.h"

#include "../../../network/messages/trust_lines/AuditMessage.h"

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

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        Initialisation = 1,
        ResponseProcessing = 2,
    };

protected: // log
    const string logHeader() const;

private:
    TransactionResult::SharedConst runInitialisationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

    pair<BytesShared, size_t> getOwnSerializedAuditData();

    pair<BytesShared, size_t> getContractorSerializedAuditData();

private:
    static const uint32_t kWaitMillisecondsForResponse = 5000;

protected:
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;
    pair<lamport::Signature::Shared, KeyNumber> mOwnSignatureAndKeyNumber;
};


#endif //GEO_NETWORK_CLIENT_INITIALAUDITSOURCETRANSACTION_H
