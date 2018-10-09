#ifndef GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGTARGETTRANSACTION_H
#define GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGTARGETTRANSACTION_H

#include "base/BaseTrustLineTransaction.h"

class PublicKeysSharingTargetTransaction : public BaseTrustLineTransaction {

public:
    typedef shared_ptr<PublicKeysSharingTargetTransaction> Shared;

public:
    PublicKeysSharingTargetTransaction(
        const NodeUUID &nodeUUID,
        PublicKeysSharingInitMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    PublicKeysSharingTargetTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    TransactionResult::SharedConst run();

protected: // log
    const string logHeader() const;

private:
    TransactionResult::SharedConst runReceiveNextKeyStage();

    TransactionResult::SharedConst runRecoveryStage();

    pair<BytesShared, size_t> serializeToBytes() const override;
};


#endif //GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGTARGETTRANSACTION_H
