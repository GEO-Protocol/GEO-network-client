#ifndef GEO_NETWORK_CLIENT_AUDITSOURCETRANSACTION_H
#define GEO_NETWORK_CLIENT_AUDITSOURCETRANSACTION_H

#include "base/BaseTrustLineTransaction.h"

class AuditSourceTransaction : public BaseTrustLineTransaction {

public:
    typedef shared_ptr<AuditSourceTransaction> Shared;

public:
    AuditSourceTransaction(
        ContractorID contractorID,
        const SerializedEquivalent equivalent,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        FeaturesManager *featuresManager,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    AuditSourceTransaction(
        const SerializedEquivalent equivalent,
        ContractorsManager *contractorsManager,
        ContractorID contractorID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        FeaturesManager *featuresManager,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runAuditPendingStage();

    TransactionResult::SharedConst runNextAttemptStage();

    TransactionResult::SharedConst runNextAttemptAuditPendingStage();

    TransactionResult::SharedConst runResponseProcessingStage();

    TransactionResult::SharedConst runContractorPendingStage();

    TransactionResult::SharedConst initializeAudit();

    TransactionResult::SharedConst nextAttemptAudit();

private:
    uint16_t mCountSendingAttempts;
    uint16_t mCountPendingAttempts;
    uint16_t mCountContractorPendingAttempts;
};


#endif //GEO_NETWORK_CLIENT_AUDITSOURCETRANSACTION_H
