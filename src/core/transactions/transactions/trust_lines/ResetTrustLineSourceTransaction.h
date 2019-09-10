#ifndef GEO_NETWORK_CLIENT_RESETTRUSTLINESOURCETRANSACTION_H
#define GEO_NETWORK_CLIENT_RESETTRUSTLINESOURCETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines/ResetTrustLineCommand.h"
#include "../../../contractors/ContractorsManager.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/keychain.h"

#include "../../../network/messages/trust_lines/TrustLineResetMessage.h"
#include "../../../network/messages/trust_lines/TrustLineConfirmationMessage.h"

class ResetTrustLineSourceTransaction : public BaseTransaction  {

public:
    typedef shared_ptr<ResetTrustLineSourceTransaction> Shared;

public:
    ResetTrustLineSourceTransaction(
        ResetTrustLineCommand::Shared command,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    enum Stages {
        Initialization = 1,
        ResponseProcessing = 2,
    };

protected:
    const string logHeader() const override;

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultUnexpectedError();

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

protected:
    static const uint32_t kWaitMillisecondsForResponse = 20000;
    static const uint16_t kMaxCountSendingAttempts = 3;

private:
    ResetTrustLineCommand::Shared mCommand;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;

    ContractorID mContractorID;
    uint16_t mCountSendingAttempts;
};

#endif //GEO_NETWORK_CLIENT_RESETTRUSTLINESOURCETRANSACTION_H
