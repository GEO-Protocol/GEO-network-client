#ifndef GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H
#define GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H

#include "base/BaseTrustLineTransaction.h"

class AuditTargetTransaction : public BaseTrustLineTransaction {

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
    TransactionResult::SharedConst sendErrorConfirmation(
        ConfirmationMessage::OperationState errorState) override;
};


#endif //GEO_NETWORK_CLIENT_AUDITTARGETTRANSACTION_H
