#ifndef GEO_NETWORK_CLIENT_INITIALAUDITSOURCETRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIALAUDITSOURCETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/KeyChain.h"

#include "../../../network/messages/trust_lines/AuditMessage.h"

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

    pair<BytesShared, size_t> serializeAuditData();

    bool deserializeAuditDataAndCheck(
        BytesShared serializedData);

private:
    static const uint32_t kWaitMillisecondsForResponse = 5000;

protected:
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    KeyChain mKeyChain;

    BytesShared ownSignedData;
    size_t ownSignedDataSize;
    uint32_t ownKeyNumber;
};


#endif //GEO_NETWORK_CLIENT_INITIALAUDITSOURCETRANSACTION_H
