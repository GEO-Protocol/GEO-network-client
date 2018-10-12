#ifndef GEO_NETWORK_CLIENT_CHECKTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CHECKTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "AuditSourceTransaction.h"
#include "PublicKeysSharingSourceTransaction.h"

class CheckTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<CheckTrustLineTransaction> Shared;

public:
    CheckTrustLineTransaction(
        const NodeUUID &nodeUUID,
        const SerializedEquivalent equivalent,
        const NodeUUID &contractorUUID,
        bool isActionInitiator,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    TransactionResult::SharedConst run();

protected: // log
    const string logHeader() const
    noexcept;

private:
    const NodeUUID mContractorUUID;
    bool mIsActionInitiator;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;
    TrustLinesInfluenceController *mTrustLinesInfluenceController;
};


#endif //GEO_NETWORK_CLIENT_CHECKTRUSTLINETRANSACTION_H
