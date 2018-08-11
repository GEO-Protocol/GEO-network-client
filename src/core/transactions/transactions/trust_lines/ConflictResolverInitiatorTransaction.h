#ifndef GEO_NETWORK_CLIENT_CONFLICTRESOLVERINITIATORTRANSACTION_H
#define GEO_NETWORK_CLIENT_CONFLICTRESOLVERINITIATORTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/keychain.h"
#include "../../../crypto/lamportkeys.h"

#include "../../../subsystems_controller/TrustLinesInfluenceController.h"

#include "../../../network/messages/trust_lines/ConflictResolverMessage.h"
#include "../../../network/messages/trust_lines/ConflictResolverResponseMessage.h"

class ConflictResolverInitiatorTransaction : public BaseTransaction  {

public:
    typedef shared_ptr<ConflictResolverInitiatorTransaction> Shared;

public:
    ConflictResolverInitiatorTransaction(
        const NodeUUID &nodeUUID,
        SerializedEquivalent equivalent,
        const NodeUUID &contractorUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    ConflictResolverInitiatorTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        Initialization = 1,
        ResponseProcessing = 2,
        Recovery = 3,
    };

protected: // log
    const string logHeader() const
    noexcept;

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

    TransactionResult::SharedConst runRecoveryStage();

    pair<BytesShared, size_t> serializeToBytes() const override;

private:
    static const uint32_t kWaitMillisecondsForResponse = 60000;

private:
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;

    NodeUUID mContractorUUID;

    TrustLinesInfluenceController *mTrustLinesInfluenceController;
};


#endif //GEO_NETWORK_CLIENT_CONFLICTRESOLVERINITIATORTRANSACTION_H
