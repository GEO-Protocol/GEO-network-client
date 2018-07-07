#ifndef GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H
#define GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../network/messages/trust_lines/AuditMessage.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/keychain.h"
#include "../../../crypto/lamportkeys.h"
#include "PublicKeysSharingSourceTransaction.h"

using namespace crypto;

class AuditTargetTransaction : public BaseTransaction {

public:
    typedef shared_ptr<AuditTargetTransaction> Shared;

public:
    AuditTargetTransaction(
        const NodeUUID &nodeUUID,
        AuditMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        Logger &logger);

    TransactionResult::SharedConst run();

protected: // log
    const string logHeader() const;

private:
    pair<BytesShared, size_t> getOwnSerializedAuditData();

    pair<BytesShared, size_t> getContractorSerializedAuditData();

protected:
    AuditMessage::Shared mMessage;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;
    pair<lamport::Signature::Shared, KeyNumber> mOwnSignatureAndKeyNumber;
    AuditNumber mAuditNumber;
};


#endif //GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H
