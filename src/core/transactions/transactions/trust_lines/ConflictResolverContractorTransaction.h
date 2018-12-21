#ifndef GEO_NETWORK_CLIENT_CONFLICTRESOLVERCONTRACTORTRANSACTION_H
#define GEO_NETWORK_CLIENT_CONFLICTRESOLVERCONTRACTORTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../contractors/ContractorsManager.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/keychain.h"
#include "../../../crypto/lamportkeys.h"

#include "../../../subsystems_controller/TrustLinesInfluenceController.h"

#include "../../../network/messages/trust_lines/ConflictResolverMessage.h"
#include "../../../network/messages/trust_lines/ConflictResolverResponseMessage.h"
#include "ConflictResolverInitiatorTransaction.h"

class ConflictResolverContractorTransaction : public BaseTransaction {

public:
    typedef shared_ptr<ConflictResolverContractorTransaction> Shared;

public:
    ConflictResolverContractorTransaction(
        const NodeUUID &nodeUUID,
        ConflictResolverMessage::Shared message,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLinesManager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    TransactionResult::SharedConst run();

protected: // log
    const string logHeader() const
    noexcept;

private:
    pair<BytesShared, size_t> getSerializedReceipt(
        ContractorID source,
        ContractorID target,
        const ReceiptRecord::Shared receiptRecord);

    void acceptContractorAuditData(
        IOTransaction::Shared ioTransaction,
        TrustLineKeychain *keyChain);

private:
    ConflictResolverMessage::Shared mMessage;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;

    ContractorID mContractorID;

    TrustLinesInfluenceController *mTrustLinesInfluenceController;
};


#endif //GEO_NETWORK_CLIENT_CONFLICTRESOLVERCONTRACTORTRANSACTION_H
